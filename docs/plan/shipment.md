# PLAN: 출고 처리 (Shipment)

> **관련 문서**
> - PRD: [docs/prd/shipment.md](../prd/shipment.md)
> - Design: [docs/design/shipment.md](../design/shipment.md)

---

## 1. 역할과 위치

출고 처리는 두 Repository를 순서대로 조율하는 단순한 흐름이다.

**결정:** 별도 Service 레이어 없이 `ShipmentController`에서 직접 처리.

**미분리 이유:**
- 로직이 단순하고 단일 흐름 (재사용 지점 없음)
- `OrderApprovalService`처럼 다중 컨트롤러에서 호출되는 구조가 아님

---

## 2. 출고 처리 흐름

```
ShipmentController::processShipment(orderId):
  order = IOrderRepository::findById(orderId)
  sample = ISampleRepository::findById(order.sampleId)

  // 방어 검사: 재고 음수 방지
  if sample.currentStock < order.quantity:
      View: 에러 메시지 ("재고 부족으로 출고 불가")
      return

  sample.currentStock -= order.quantity          // BR-10: 출고 시점에만 차감
  ISampleRepository::update(sample)

  order.status = RELEASE                         // BR-11: CONFIRMED → RELEASE
  IOrderRepository::update(order)

  View: 출고 완료 메시지
```

---

## 3. 출고 대상 필터링

출고 목록 조회: `IOrderRepository::findByStatus(OrderStatus::CONFIRMED)`

CONFIRMED 상태 주문만 목록에 표시하므로, `processShipment()` 진입 시 status 재검증은 생략 가능하다.
단, 재고 음수 방지 검사(`currentStock >= quantity`)는 데이터 정합성 보호를 위해 유지한다.

---

## 4. 재고 차감 원칙 (BR-10)

재고(`currentStock`) 차감은 오직 출고 처리 시점에만 발생한다.

| 이벤트 | 재고 변화 |
|--------|----------|
| 주문 승인 → CONFIRMED | 변화 없음 |
| 생산 완료 → CONFIRMED | 증가 (실 생산량 추가, ProductionService 처리) |
| 출고 처리 → RELEASE | **차감** (ShipmentController 처리) |

`ShipmentController`의 `ISampleRepository::update()`가 재고 차감의 **유일한 경로**이다.
