# PLAN: 생산 라인 (Production Line)

> **관련 문서**
> - PRD: [docs/prd/production-line.md](../prd/production-line.md)
> - Design: [docs/design/production-line.md](../design/production-line.md)

---

## 1. ProductionRepository 설계

### 1.1 왜 JsonRepository\<T\>를 사용하지 않는가

`production.json`의 구조는 엔티티 목록이 아닌 **단일 상태 객체**이다.

```json
{
  "activeJob": { "orderId": "ORD-002", "startTimeUnix": 1746691200, ... },
  "queue": [
    { "orderId": "ORD-003", ... }
  ]
}
```

`JsonRepository<T>`(엔티티 목록 + nextId 구조)는 이 형식에 맞지 않는다.

**결정:** `IProductionRepository` + `JsonProductionRepository` 전용 구현.
- `load()` → 파일에서 `ProductionState` 전체를 읽어 반환
- `save(state)` → `ProductionState` 전체를 파일에 덮어씀

```cpp
struct ProductionState {
    std::optional<ProductionJob> activeJob;
    std::deque<ProductionJob>    queue;
};

class IProductionRepository {
public:
    virtual ~IProductionRepository() = default;
    virtual ProductionState getState() const = 0;
    virtual void            setState(const ProductionState& state) = 0;
};
```

싱글턴 인스턴스가 인메모리 `ProductionState`를 유지하며, `setState()` 호출 시 파일에 즉시 flush한다.

---

## 2. ITimeProvider 인터페이스

**참조:** 마스터 PRD 섹션 5.6.3 (시간 Mocking)

모든 시간 연산은 반드시 `ITimeProvider::now()`를 경유해야 한다 (직접 `std::time(nullptr)` 호출 금지).

```cpp
class ITimeProvider {
public:
    virtual ~ITimeProvider() = default;
    virtual time_t    now() const = 0;
    virtual std::string nowIso8601() const = 0;  // 주문 createdAt 포맷용
};

class RealTimeProvider : public ITimeProvider {
    time_t now() const override { return std::time(nullptr); }
    std::string nowIso8601() const override; // <ctime> strftime 사용
};

class MockTimeProvider : public ITimeProvider {
    time_t current_;
public:
    MockTimeProvider() : current_(std::time(nullptr)) {}
    time_t now() const override { return current_; }
    void advance(int minutes)   { current_ += minutes * 60; }
    void setTime(time_t t)      { current_ = t; }
    std::string nowIso8601() const override;
};
```

`main.cpp`에서 `--mock-time` 인수 확인 후 생성할 구현체를 선택한다 (composition root 패턴).

---

## 3. ProductionService — Lazy 완료 체크

### 3.1 Service 레이어로 분리하는 이유

완료 처리 로직은 두 지점에서 호출된다:
- `AppController::run()` — 메인 루프 매 순환 (PRD: 메인 메뉴 자동 체크)
- `ProductionLineController` — 생산 라인 메뉴 진입 시

재사용이 필요하므로 Controller에 직접 넣지 않고 **Service 레이어**로 분리한다.
`SampleRepository`, `OrderRepository`, `ProductionRepository` 세 곳을 함께 조율하므로 한 레이어에 모으는 것이 SRP 원칙에도 부합한다.

```cpp
class ProductionService {
public:
    ProductionService(ISampleRepository&, IOrderRepository&,
                      IProductionRepository&, ITimeProvider&);
    void checkAndComplete();   // 외부 트리거 진입점
private:
    bool tryCompleteActiveJob();
    void startNextJob();
};
```

### 3.2 완료 체크 알고리즘 (BR-15, BR-16, BR-17)

```
checkAndComplete():
  state = IProductionRepository::getState()
  if state.activeJob is empty → return

  elapsed = ITimeProvider::now() - activeJob.startTimeUnix
  if elapsed < activeJob.totalProductionTimeMin × 60 → return  // 미완료

  // 생산 완료 처리
  sample = ISampleRepository::findById(activeJob.sampleId)
  sample.currentStock += activeJob.actualProductionQty          // BR-09: 실 생산량 전부 추가
  ISampleRepository::update(sample)

  order = IOrderRepository::findById(activeJob.orderId)
  order.status = CONFIRMED                                      // BR-09: PRODUCING → CONFIRMED
  IOrderRepository::update(order)

  state.activeJob = nullopt
  IProductionRepository::setState(state)

  if state.queue not empty:
      startNextJob()
      checkAndComplete()   // BR-17: 새로 시작된 작업도 즉시 재귀 체크
```

**재귀 체크 (BR-17) 의도:**
프로그램이 꺼져 있던 동안 큐의 여러 작업이 순차 완료될 수 있다.
`startNextJob()` 후 즉시 `checkAndComplete()`를 재귀 호출하여 연쇄 완료를 처리한다.
대기 큐가 비면 재귀가 자연스럽게 종료된다.

### 3.3 startNextJob()

```
startNextJob():
  state = IProductionRepository::getState()
  nextJob = state.queue.front()
  state.queue.pop_front()
  nextJob.startTimeUnix = ITimeProvider::now()    // 실제 시작 시각 기록
  state.activeJob = nextJob
  IProductionRepository::setState(state)
```

---

## 4. ProductionJob 도메인 구조체

```cpp
struct ProductionJob {
    std::string orderId;
    std::string sampleId;
    int         shortage               = 0;
    int         actualProductionQty    = 0;
    int         totalProductionTimeMin = 0;
    int64_t     startTimeUnix          = 0;  // queue 항목은 0, activeJob만 유효값
};
```

`startTimeUnix`는 `int64_t` (PRD 정의)로 저장하여 2038년 문제를 피한다.

---

## 5. MockTimeProvider 활성화

```
// main.cpp (composition root)
if (argc > 1 && std::string(argv[1]) == "--mock-time") {
    auto tp = std::make_shared<MockTimeProvider>();
    // ProductionLineView에 "시간 앞당기기(advance)" 메뉴 항목 추가
} else {
    auto tp = std::make_shared<RealTimeProvider>();
}
```

`--mock-time` 모드에서 `ProductionLineController`는 추가 메뉴 항목(`advance N분`)을 노출하여
`MockTimeProvider::advance(N)`을 호출할 수 있게 한다.

---

## 6. 프로그램 재시작 내구성 (BR-16)

- `activeJob.startTimeUnix`가 JSON에 저장되므로 재시작 후에도 시작 시각이 유지된다.
- 재시작 시 `AppController::run()` 첫 루프에서 `ProductionService::checkAndComplete()`가 호출되어
  꺼져 있던 동안 완료된 작업을 자동으로 일괄 처리한다.
- 별도 "복구" 코드 불필요 — Lazy 체크가 자동으로 처리한다.
