# PLAN: 메인 메뉴 (Main Menu)

> **관련 문서**
> - PRD: [docs/prd/main-menu.md](../prd/main-menu.md)
> - Design: [docs/design/main-menu.md](../design/main-menu.md)

---

## 1. 전체 프로젝트 아키텍처 결정

> 이 섹션은 메인 메뉴뿐 아니라 전체 프로젝트의 기술 방향을 결정한다.
> 각 기능 PLAN은 여기서 확정된 사항을 전제로 작성된다.

### 1.1 MVC 구조 — ConsoleMVC PoC 적용

**참조:** [ConsoleMVC PoC](https://github.com/bongsujeon2-cmd/ConsoleMVC-bongsujeon-12250769)

PoC의 의존 방향과 인터페이스 분리 원칙을 그대로 적용한다.

```
의존 방향: Controller → Model(인터페이스), Controller → IView
           Model 변경 → IObserver → View (Observer 패턴)
```

`main.cpp`는 **composition root 역할만** 수행한다 (PoC `main.cpp` 패턴).
- Repository, Service, View, Controller를 생성하고 조립
- 비즈니스 로직 없음
- `--mock-time` 인수 분기도 여기서 처리

### 1.2 디렉토리 구조

```
SampleOrderSystem/
├── Model/
│   ├── Domain/           # 도메인 구조체 (Sample, Order, ProductionJob)
│   ├── Repository/       # 인터페이스 + JSON 구현체
│   └── Service/          # ProductionService, ITimeProvider 계열
├── View/                 # 기능별 View 클래스 (IView 구현)
├── Controller/           # 기능별 Controller 클래스 (IController 구현)
├── Tools/                # DataMonitorTool, SemiDummyGenerator
├── json.hpp              # DataPersistence PoC의 커스텀 JsonValue (복사)
└── main.cpp
data/
├── samples.json
├── orders.json
└── production.json
test/                     # Google Mock 테스트 파일
```

### 1.3 json.hpp 선택

**결정:** DataPersistence PoC의 커스텀 `JsonValue` (`json.hpp`)를 직접 복사해 사용한다.

| 항목 | PoC json.hpp | nlohmann/json |
|------|-------------|--------------|
| 크기 | ~10KB | ~800KB |
| 외부 의존 | 없음 | 없음 (단일 헤더) |
| 기능 범위 | CRUD + 파일 I/O 충분 | 훨씬 많음 |
| 검증 여부 | PoC에서 검증됨 | 업계 표준 |

PoC에서 이미 `parseFile`, `saveToFile`, `stringify`, `at`, `contains` 등 필요한 API가 검증되었다. 마스터 PRD의 "nlohmann/json" 언급은 단일 헤더 JSON 라이브러리를 의미하며, PoC의 커스텀 구현이 동등한 역할을 한다.

**트레이드오프:** 복잡한 JSON 스키마 validation 기능 없음 → 본 프로젝트 요구사항 범위 내에서 불필요.

### 1.4 Repository 싱글턴 전략

**결정:** Repository 인스턴스는 `main.cpp`에서 한 번 생성하여 모든 Controller/Service에 레퍼런스로 전달한다.

- 생성자에서 JSON 파일을 로드하여 인메모리 캐시 유지
- 쓰기 연산(create/update/remove)은 인메모리 수정 + 즉시 파일 flush
- 읽기 연산은 인메모리에서 반환 (빠름)
- 단일 스레드 콘솔 앱이므로 동시성 문제 없음

**이유:** PoC `JsonRepository<T>`의 패턴과 동일. 각 컨트롤러가 파일을 독립적으로 열면 변경 사항이 충돌할 위험이 있다.

---

## 2. 컨트롤러 구성

| 컨트롤러 | 담당 기능 |
|---------|----------|
| `AppController` | 메인 루프, 메뉴 라우팅, Lazy 완료 체크 트리거 |
| `SampleController` | 시료 등록/조회/검색 |
| `OrderController` | 주문 접수, 승인/거절 |
| `ShipmentController` | 출고 처리 |
| `MonitoringController` | 주문량/재고량 집계 조회 |
| `ProductionLineController` | 생산 현황/대기 큐 조회 |

---

## 3. 메인 루프와 Lazy 완료 체크 통합

### 3.1 설계 결정

PRD 요구: 메인 루프 순환마다 `activeJob` 완료 여부를 자동 체크.

**결정:** `AppController::run()` 루프의 **메뉴 표시 직전**에 `ProductionService::checkAndComplete()` 호출.

```
while (true):
    ProductionService::checkAndComplete()   // Lazy 체크 (항상 먼저)
    View: 재고 스냅샷 + 메뉴 출력
    choice = View: 입력 받기
    서브컨트롤러 라우팅
```

**트레이드오프:**
- 메뉴가 표시될 때마다 최신 상태 보장
- 별도 스레드/타이머 없이 단순 구조 유지
- `ProductionService`가 여러 Repository를 조율하므로 `AppController`가 직접 Repository를 건드리지 않음

### 3.2 재고 스냅샷 표시

`SampleRepository::findAll()`로 전체 시료를 읽어 View에 전달.
표시 정보: 시료 ID, 이름, 현재 재고 (조회 시점 기준).

---

## 4. 인터페이스 전략 (테스트 대응)

각 Repository와 TimeProvider에 순수 가상 인터페이스를 정의해 Google Mock 교체를 가능하게 한다.

```
ISampleRepository      ← JsonSampleRepository (실제), MockSampleRepository (테스트)
IOrderRepository       ← JsonOrderRepository (실제), MockOrderRepository (테스트)
IProductionRepository  ← JsonProductionRepository (실제)
ITimeProvider          ← RealTimeProvider (실제), MockTimeProvider (--mock-time)
```

인터페이스는 `Model/Repository/` 하위에 헤더로 정의한다.
