# S-Semi 반도체 시료 생산주문관리 시스템

가상의 반도체 회사 **S-Semi**가 연구소·팹리스 업체·대학 연구실에 반도체 시료(Sample)를 생산·납품하는 콘솔 애플리케이션.

---

## 목차

- [요구 환경](#요구-환경)
- [빌드](#빌드)
- [실행](#실행)
- [테스트](#테스트)
- [관리자 도구](#관리자-도구)
- [프로젝트 구조](#프로젝트-구조)
- [주문 상태 흐름](#주문-상태-흐름)
- [데이터 파일](#데이터-파일)
- [문서](#문서)

---

## 요구 환경

| 항목 | 버전 |
|------|------|
| Visual Studio | 2022 (v145 toolset) |
| C++ 표준 | C++20 |
| OS | Windows 10/11 x64 |
| Google Mock | 1.11.0 (NuGet, 자동 복원) |

---

## 빌드

솔루션 루트(`SampleOrderSystem.slnx`)가 있는 디렉토리에서 실행합니다.

```powershell
# Debug 빌드
msbuild SampleOrderSystem.slnx /p:Configuration=Debug /p:Platform=x64

# Release 빌드
msbuild SampleOrderSystem.slnx /p:Configuration=Release /p:Platform=x64
```

빌드 결과물 위치: `x64\Debug\SampleOrderSystem.exe`

> **NuGet 패키지**: Google Mock 1.11.0이 `packages/gmock.1.11.0/`에 포함되어 있으므로 별도 복원이 필요 없습니다.

---

## 실행

모든 명령은 **솔루션 루트 디렉토리**에서 실행합니다 (`data/` 폴더 상대 경로 기준).

### 메인 프로그램

```powershell
.\x64\Debug\SampleOrderSystem.exe
```

실행 시 메인 메뉴가 표시되며, 번호를 입력해 기능을 선택합니다.

```
======= S-Semi 반도체 시료 생산주문관리 =======
[시료 재고 현황]
  S-001 AlGaN    재고: 50개
  ...
[메뉴]
  [1] 시료 관리
  [2] 주문
  [3] 모니터링
  [4] 출고 처리
  [5] 생산 라인
  [0] 종료
```

### 시간 Mocking 모드 (`--mock-time`)

실제 분 단위 대기 없이 생산 완료를 빠르게 검증하는 모드입니다.

```powershell
.\x64\Debug\SampleOrderSystem.exe --mock-time
```

- 생산 라인 메뉴에 **[3] 시간 앞당기기** 항목이 추가됩니다.
- 입력한 분(minutes)만큼 내부 시각을 앞당겨 생산 완료 조건을 즉시 트리거합니다.

---

## 테스트

Google Mock/Test 기반 단위 테스트를 `--test` 플래그로 실행합니다.

```powershell
.\x64\Debug\SampleOrderSystem.exe --test
```

**테스트 결과 예시:**

```
[==========] 102 tests from 11 test suites ran.
[  PASSED  ] 102 tests.
```

### 테스트 스위트 목록

| 스위트 | 테스트 수 | 검증 내용 |
|--------|-----------|-----------|
| `SampleRepositoryTest` | 12 | JsonSampleRepository CRUD·영속성·검색 |
| `SampleControllerTest` | 8 | 시료 등록 검증·목록·검색 |
| `OrderRepositoryTest` | 12 | ORD-NNN ID 생성·상태별 조회·영속성 |
| `OrderControllerTest` | 12 | 주문 접수·승인 분기(BR-06/07)·거절 |
| `ProductionRepositoryTest` | 9 | ProductionState FIFO·영속성 |
| `ProductionServiceTest` | 8 | Lazy 완료(BR-15/16)·연쇄 처리(BR-17) |
| `ProductionLineControllerTest` | 5 | 생산 현황·대기 큐·시간 Mocking |
| `ShipmentControllerTest` | 7 | 출고 처리(BR-10/11)·재고 차감 |
| `MonitoringControllerTest` | 10 | 주문량 집계(BR-12)·재고 상태 판단 |
| `AppControllerTest` | 9 | 메인 루프·Lazy 체크·메뉴 라우팅 |
| `SemiDummyGeneratorTest` | 10 | BR-01·06·07·08·13·14 준수 검증 |

---

## 관리자 도구

개발·테스트 단계에서 데이터를 빠르게 확인하고 초기화하기 위한 도구입니다.

### 데이터 모니터링 (`--dump-data`)

현재 JSON 파일에 저장된 모든 데이터를 콘솔에 출력합니다.

```powershell
.\x64\Debug\SampleOrderSystem.exe --dump-data
```

`data/samples.json`, `data/orders.json`, `data/production.json` 세 파일의 내용을 테이블 형식으로 표시합니다.

### 더미 데이터 생성 (`--gen-dummy`)

테스트용 시나리오 데이터를 자동 생성합니다. PRD 비즈니스 규칙(BR-01~BR-18)을 준수합니다.

```powershell
# 기존 데이터 초기화 후 새 데이터 생성
.\x64\Debug\SampleOrderSystem.exe --gen-dummy

# 기존 데이터에 추가
.\x64\Debug\SampleOrderSystem.exe --gen-dummy --append
```

**생성 시나리오:**
- 시료 5~10개 (yieldRate 0.70~1.00, avgProductionTime 3~20분)
- 주문 10~20개 (RESERVED 20% / PRODUCING 30% / CONFIRMED 50%)
- production.json: PRODUCING 주문의 activeJob + queue 연동

**전형적인 E2E 개발 흐름:**

```powershell
# 1. 더미 데이터 생성
.\x64\Debug\SampleOrderSystem.exe --gen-dummy

# 2. 데이터 확인
.\x64\Debug\SampleOrderSystem.exe --dump-data

# 3. 메인 프로그램 실행 (mock-time으로 생산 완료 빠른 확인)
.\x64\Debug\SampleOrderSystem.exe --mock-time

# 4. 처리 결과 확인
.\x64\Debug\SampleOrderSystem.exe --dump-data
```

---

## 프로젝트 구조

```
SampleOrderSystem/
├── Model/
│   ├── Domain/              # 도메인 구조체
│   │   ├── Sample.h         # 시료 (yieldRate, avgProductionTime, currentStock)
│   │   ├── Order.h          # 주문 + OrderStatus 열거형
│   │   ├── ProductionJob.h  # 생산 작업 + ProductionState
│   │   └── StockStatus.h    # 재고 상태 (SURPLUS/SHORTAGE/DEPLETED)
│   ├── Repository/          # 인터페이스 + JSON 구현체
│   │   ├── ISampleRepository.h / JsonSampleRepository.h
│   │   ├── IOrderRepository.h  / JsonOrderRepository.h
│   │   └── IProductionRepository.h / JsonProductionRepository.h
│   └── Service/             # 비즈니스 서비스
│       ├── ITimeProvider.h  / RealTimeProvider.h / MockTimeProvider.h
│       ├── IProductionService.h / ProductionService.h/.cpp
│       └── TimeUtils.h
├── View/                    # 콘솔 입출력 (인터페이스 + 구현체)
│   ├── ISampleView.h / SampleView.h/.cpp
│   ├── IOrderView.h / OrderView.h/.cpp
│   ├── IMonitoringView.h / MonitoringView.h/.cpp
│   ├── IShipmentView.h / ShipmentView.h/.cpp
│   ├── IProductionLineView.h / ProductionLineView.h/.cpp
│   └── IMainMenuView.h / MainMenuView.h/.cpp
├── Controller/              # 기능별 컨트롤러
│   ├── ISubController.h     # run() 인터페이스 (AppController 디스패처용)
│   ├── AppController.h/.cpp # 메인 루프 + Lazy 체크
│   ├── SampleController.h/.cpp
│   ├── OrderController.h/.cpp
│   ├── MonitoringController.h/.cpp
│   ├── ShipmentController.h/.cpp
│   └── ProductionLineController.h/.cpp
├── Tools/                   # 관리자 도구
│   ├── DataMonitorTool.h/.cpp
│   └── SemiDummyGenerator.h/.cpp
├── test/                    # Google Mock 단위 테스트
│   ├── SampleRepositoryTest.cpp
│   ├── SampleControllerTest.cpp
│   ├── OrderRepositoryTest.cpp
│   ├── OrderControllerTest.cpp
│   ├── ProductionRepositoryTest.cpp
│   ├── ProductionServiceTest.cpp
│   ├── ProductionLineControllerTest.cpp
│   ├── ShipmentControllerTest.cpp
│   ├── MonitoringControllerTest.cpp
│   ├── AppControllerTest.cpp
│   └── SemiDummyGeneratorTest.cpp
├── json.hpp                 # 경량 커스텀 JSON 파서 (BOM 처리 포함)
└── main.cpp                 # Composition Root

data/                        # 런타임 데이터 (JSON 영속성)
├── samples.json
├── orders.json
└── production.json
```

---

## 주문 상태 흐름

```
[주문 접수]
    │
    ▼
RESERVED ──────────────► REJECTED
    │           (거절)
    │ (승인)
    ├─── 재고 충분 ────────► CONFIRMED ──► RELEASE
    │                                   (출고+재고차감)
    └─── 재고 부족 ────────► PRODUCING ──► CONFIRMED ──► RELEASE
                           (생산 중)     (생산 완료)   (출고+재고차감)
```

- **RESERVED**: 주문 접수 대기
- **PRODUCING**: 재고 부족으로 생산 중 (생산 큐 FIFO)
- **CONFIRMED**: 출고 대기
- **RELEASE**: 출고 완료
- **REJECTED**: 거절 (모니터링 집계 제외)

---

## 데이터 파일

모든 데이터는 `data/` 디렉토리의 JSON 파일에 저장됩니다. 앱 실행 시 디렉토리가 자동 생성됩니다.

| 파일 | 내용 |
|------|------|
| `data/samples.json` | 시료 목록 (ID, 이름, 수율, 평균생산시간, 재고) |
| `data/orders.json` | 주문 목록 + nextOrdNum 카운터 |
| `data/production.json` | 현재 생산 작업(activeJob) + 대기 큐(queue) |

> `production.json`의 `activeJob.startTimeUnix`는 Unix timestamp입니다. 프로그램이 꺼져 있던 동안에도 시간이 흐르므로, 재시작 시 자동으로 완료 처리됩니다 (BR-16).

---

## 문서

| 문서 | 경로 | 내용 |
|------|------|------|
| 마스터 PRD | `docs/master_prd.md` | 전체 요구사항 및 비즈니스 규칙(BR-01~BR-18) |
| 기능별 PRD | `docs/prd/` | 각 기능의 사용자 스토리·성공 기준 |
| PLAN | `docs/plan/` | 기술 방식 결정·아키텍처·트레이드오프 |
| Design | `docs/design/` | Phase별 구현 태스크·검증 방법 |
| 커밋 가이드 | `docs/guide/commit-guide.md` | 커밋 메시지 규칙 |
