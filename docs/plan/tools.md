# PLAN: 관리자 도구 (Tools)

> **관련 문서**
> - PRD: [docs/prd/tools.md](../prd/tools.md)
> - Design: [docs/design/tools.md](../design/tools.md)

---

## 1. 도구 통합 방식

**결정:** 메인 실행 파일(`SampleOrderSystem.exe`)의 **커맨드라인 인수**로 활성화.

```
SampleOrderSystem.exe                → 메인 프로그램 (정상 실행)
SampleOrderSystem.exe --dump-data    → 데이터 모니터링 Tool
SampleOrderSystem.exe --gen-dummy    → 더미 데이터 생성 Tool (기존 데이터 덮어쓰기)
SampleOrderSystem.exe --gen-dummy --append → 더미 데이터 추가 생성
SampleOrderSystem.exe --mock-time    → 시간 Mocking 모드 (메인 프로그램)
```

**이유:**
- 현재 솔루션은 단일 프로젝트(`SampleOrderSystem.vcxproj`)이므로 별도 실행 파일 추가 없이 유지
- PoC에서는 독립 실행 파일(DataMonitor, DummyDataGenerator)이었으나, 본 프로젝트는 동일 `data/` 폴더를 공유하므로 통합이 더 일관적
- `main.cpp`에서 인수를 분기하여 해당 Tool 클래스의 `run()`을 호출

```cpp
// main.cpp 진입점 분기
int main(int argc, char* argv[]) {
    if (hasArg(argc, argv, "--dump-data"))  return DataMonitorTool().run();
    if (hasArg(argc, argv, "--gen-dummy"))  return SemiDummyGenerator().run(hasArg(argc, argv, "--append"));
    bool mockTime = hasArg(argc, argv, "--mock-time");
    // ... 메인 프로그램 실행
}
```

---

## 2. DataMonitorTool

### 2.1 DataMonitor PoC 적용 방식

**참조:** [DataMonitor PoC](https://github.com/bongsujeon2-cmd/DataMonitor-bongsujeon-12250769)

PoC는 범용 User/Product CRUD 모니터링 도구다. S-Semi 도메인에 맞게 다음과 같이 대응한다.

| PoC 메뉴 | S-Semi 적용 |
|---------|-----------|
| `userMenu()` | `sampleMenu()` — samples.json 시료 목록/검색 조회 |
| `productMenu()` | `orderMenu()` — orders.json 주문 상태별 조회 |
| 없음 | `productionMenu()` — production.json activeJob/queue 조회 |

### 2.2 PoC에서 재사용하는 패턴

- `getChoice(min, max)` — 메뉴 입력 루프
- `pause()` — "Enter to continue" 대기
- `separator(title, sub)` — 구분선 + 제목 출력
- `toLower()` — 대소문자 무시 검색
- 테이블 출력: `std::setw` + `std::left` 컬럼 정렬

### 2.3 Repository 재사용

`DataMonitorTool`은 실제 `JsonSampleRepository`, `JsonOrderRepository`, `JsonProductionRepository` 인스턴스를 직접 생성하여 사용한다 (메인 프로그램의 싱글턴과 별개 인스턴스, 파일은 동일).

**읽기 전용 사용** 원칙: Tool에서는 데이터 수정 없이 조회만 수행한다 (--dump-data 의미에 충실).

---

## 3. SemiDummyGenerator

### 3.1 PoC SchemaGenerator를 그대로 사용하지 않는 이유

**참조:** [DummyDataGenerator PoC](https://github.com/bongsujeon2-cmd/DummyDataGenerator-bongsujeon-12250769)

PoC의 `SchemaGenerator`는 JSON 스키마 기반 범용 랜덤 생성기다.
S-Semi의 비즈니스 규칙(BR-01~BR-18)을 모르므로 그대로 사용하면:
- 존재하지 않는 sampleId를 가진 Order가 생성될 수 있음
- PRODUCING 주문이 production.json과 연동되지 않을 수 있음
- yieldRate가 유효 범위(0 < y ≤ 1.0)를 벗어날 수 있음

**결정:** **도메인 전용 `SemiDummyGenerator` 클래스** 구현.
- PoC의 랜덤 유틸리티(`std::mt19937`, `uniform_int_distribution`, `uniform_real_distribution`)와 풀(pick) 패턴은 참고하여 재사용
- 생성 로직은 BR 준수를 명시적으로 보장하는 고정 시나리오

### 3.2 생성 시나리오

```
1단계: 시료 생성 (5~10개)
  - id: "S-001" ~ "S-010" (순번)
  - name: 반도체 관련 고정 풀에서 선택 (예: "AlGaN", "GaN", "SiC", ...)
  - avgProductionTime: 3~30분 (랜덤)
  - yieldRate: 0.70~1.00 (소수점 2자리)
  - currentStock: 10~100 (랜덤)

2단계: 주문 생성 (10~20개)
  - 각 주문의 sampleId는 1단계에서 생성된 시료 중에서 선택 (BR-01 보장)
  - 상태 비율: RESERVED 20% / PRODUCING 30% / CONFIRMED 50%
  - CONFIRMED 주문: 현재 재고 >= 주문 수량 (BR-03 시뮬레이션)
  - PRODUCING 주문: 현재 재고 < 주문 수량 (부족 시뮬레이션)
  - 생산 수치는 BR-06, BR-07 공식으로 계산

3단계: 생산 상태 생성
  - PRODUCING 주문 중 첫 번째 → activeJob (startTimeUnix = 현재 - N분, N < totalProductionTimeMin)
  - 나머지 PRODUCING 주문 → queue (FIFO 순서, BR-08)
  - activeJob, queue 모두 production.json에 기록
  - PRODUCING 주문이 없으면 activeJob = null, queue = []
```

**BR 준수 체크리스트:**

| BR | 적용 방법 |
|----|----------|
| BR-01 | Order.sampleId는 생성된 Sample 목록에서만 선택 |
| BR-06 | `ceil(shortage / (yieldRate × 0.9))` 공식 적용 |
| BR-07 | `avgProductionTime × actualProductionQty` 공식 적용 |
| BR-08 | queue는 주문 생성 순서(FIFO) 유지 |
| BR-13 | activeJob은 최대 1개 |
| BR-14 | yieldRate ∈ (0, 1.0] 범위 내 생성 |

### 3.3 출력 파일

| 파일 | 생성 내용 |
|------|---------|
| `data/samples.json` | 시료 목록 (PRD 스키마 형식) |
| `data/orders.json` | 주문 목록 (nextOrdNum 포함) |
| `data/production.json` | activeJob + queue |

`--append` 플래그가 없으면 기존 파일 덮어쓰기 (실행 전 확인 메시지 출력).

---

## 4. Tools 소스 구조

```
SampleOrderSystem/Tools/
├── DataMonitorTool.h
├── DataMonitorTool.cpp
├── SemiDummyGenerator.h
└── SemiDummyGenerator.cpp
```
