# PLAN: 주문 접수 및 승인/거절 (Order)

> **관련 문서**
> - PRD: [docs/prd/order.md](../prd/order.md)
> - Design: [docs/design/order.md](../design/order.md)

---

## 1. OrderRepository 설계

### 1.1 ORD-NNN ID 생성 전략

주문 ID는 `ORD-NNN` 형식의 자동 생성 string이다.
DataPersistence PoC의 `JsonRepository<T>` 는 `int64_t` 자동증가 ID를 사용하므로 그대로 쓸 수 없다.

**결정:** `IOrderRepository` + `JsonOrderRepository` 직접 구현.
- `data/orders.json`에 `nextOrdNum` 정수 필드를 저장하여 번호를 영속 관리
- ID 생성: `"ORD-" + zero-padded(nextOrdNum++, 3)` (예: `ORD-001`)

```json
{
  "nextOrdNum": 4,
  "orders": [
    {
      "id": "ORD-001",
      "sampleId": "S-001",
      "customerName": "삼성연구소",
      "quantity": 20,
      "status": "RESERVED",
      "createdAt": "2026-05-08T10:00:00"
    }
  ]
}
```

**PoC 대비 차이점:** PoC `JsonRepository`의 `nextId`는 정수지만, 우리는 `nextOrdNum`을 정수로 저장하고 string 형식(`ORD-NNN`)으로 변환한다. 저장 구조는 유사하다.

### 1.2 IOrderRepository 인터페이스

```cpp
class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;
    virtual std::string      create(Order order) = 0;   // 반환: 할당된 id
    virtual std::optional<Order> findById(const std::string& id) const = 0;
    virtual std::vector<Order>   findAll() const = 0;
    virtual std::vector<Order>   findByStatus(OrderStatus status) const = 0;
    virtual bool             update(const Order& order) = 0;
};
```

---

## 2. Order 도메인 구조체 및 상태 열거형

```cpp
enum class OrderStatus {
    RESERVED, PRODUCING, CONFIRMED, RELEASE, REJECTED
};

struct Order {
    std::string  id;
    std::string  sampleId;
    std::string  customerName;
    int          quantity    = 0;
    OrderStatus  status      = OrderStatus::RESERVED;
    std::string  createdAt;          // ISO 8601: "2026-05-08T10:00:00"

    JsonValue toJson() const;
    static Order fromJson(const JsonValue& j);
};

std::string     toString(OrderStatus s);
OrderStatus     fromString(const std::string& s);
```

---

## 3. 주문 승인 로직 설계

### 3.1 위치: OrderController

승인 처리는 `ISampleRepository`와 `IProductionRepository`를 함께 조율해야 한다.

**결정:** `OrderController::approveOrder()` 에서 직접 조율한다.

**`OrderApprovalService` 별도 분리를 검토했으나 채택하지 않은 이유:**
- 승인 로직은 `OrderController`에서만 호출됨 (재사용 지점 없음)
- 레이어를 추가하면 클래스 수만 늘고 이점 없음 (YAGNI)

### 3.2 승인 흐름

```
OrderController::approveOrder(orderId):
  order = IOrderRepository::findById(orderId)
  sample = ISampleRepository::findById(order.sampleId)

  if sample.currentStock >= order.quantity:
      order.status = CONFIRMED
      IOrderRepository::update(order)
      // 재고 차감 없음 — 출고(RELEASE) 시점에만 차감 (BR-03, BR-10)

  else:
      shortage = order.quantity - sample.currentStock
      actualProductionQty = ceil(shortage / (sample.yieldRate * 0.9))    // BR-06
      totalProductionTimeMin = sample.avgProductionTime * actualProductionQty // BR-07

      job = ProductionJob {
          orderId              = order.id,
          sampleId             = order.sampleId,
          shortage             = shortage,
          actualProductionQty  = actualProductionQty,
          totalProductionTimeMin = totalProductionTimeMin,
          startTimeUnix        = 0  // 큐 대기 상태
      }

      IProductionRepository::enqueue(job)    // BR-04, BR-08
      order.status = PRODUCING
      IOrderRepository::update(order)
```

### 3.3 생산량 계산 공식

```cpp
int shortage    = order.quantity - sample.currentStock;
int actualQty   = static_cast<int>(
                    std::ceil(shortage / (sample.yieldRate * 0.9)));
int totalMinutes = sample.avgProductionTime * actualQty;
```

- `<cmath>` 의 `std::ceil()` 사용
- BR-06 예시 검증: 부족분=10, 수율=0.9 → ceil(10/(0.9×0.9)) = ceil(12.345) = **13**

---

## 4. 접수 일시 (createdAt)

- `ITimeProvider::nowIso8601()` 메서드 추가 (또는 `now()` 반환값을 포맷 변환)
- 형식: `"YYYY-MM-DDTHH:MM:SS"` (ISO 8601 로컬 시각)
- MockTimeProvider도 동일 형식으로 반환

---

## 5. 거절 처리

```
OrderController::rejectOrder(orderId):
  order = IOrderRepository::findById(orderId)
  order.status = REJECTED
  IOrderRepository::update(order)
  // 추가 작업 없음 — 모니터링에서 자동 제외 (BR-12)
```

거절된 주문은 `production.json`에 영향 없다. RESERVED 상태에서만 거절 가능 (BR-02).
