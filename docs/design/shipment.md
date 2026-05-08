# Design: 출고 처리 (Shipment)

> **관련 문서**
> - PRD: [docs/prd/shipment.md](../prd/shipment.md)
> - PLAN: [docs/plan/shipment.md](../plan/shipment.md)

> **선행 조건:** `order` Design의 모든 Phase 완료 (CONFIRMED 상태 주문 존재 가능)

---

## Phase 1: ShipmentController 구현

### Tasks

#### 테스트 코드
- [ ] `test/ShipmentControllerTest.cpp` 작성 (Mock 레포지토리 사용)

  **정상 출고**
  - `processShipment(orderId)`: `ISampleRepository::update()` 호출 확인 (재고 차감)
  - 차감량 = `order.quantity` 확인 (BR-10)
  - `IOrderRepository::update()` 호출, `order.status = RELEASE` 확인 (BR-11)

  **재고 부족 방어**
  - `sample.currentStock < order.quantity` → 에러 반환, update 미호출 확인

  **목록 필터링**
  - `listConfirmedOrders()`: `findByStatus(CONFIRMED)` 결과만 반환 확인

#### 구현
- [ ] `Controller/ShipmentController.h/.cpp`
  - `listConfirmedOrders()`: `IOrderRepository::findByStatus(CONFIRMED)` → View 렌더링
  - `processShipment(orderId)`:
    1. `IOrderRepository::findById(orderId)` → order 조회
    2. `ISampleRepository::findById(order.sampleId)` → sample 조회
    3. `currentStock < quantity` 체크 → 에러 처리
    4. `sample.currentStock -= order.quantity` + `ISampleRepository::update(sample)`
    5. `order.status = RELEASE` + `IOrderRepository::update(order)`

### 검증
- 모든 테스트 PASS

---

## Phase 2: ShipmentView 구현

### Tasks

- [ ] `View/ShipmentView.h/.cpp`
  - `showSubMenu()`: [1] 출고 대기 목록 / [2] 출고 처리 / [0] 돌아가기
  - `showConfirmedOrders(orders, samples)`: 주문 ID / 시료명 / 고객명 / 수량 테이블 출력
  - `showNoConfirmedOrders()`: "출고 대기 주문 없음" 메시지
  - `promptOrderIdInput()`: 출고할 주문 ID 입력
  - `showShipmentSuccess(orderId, qty)`: 출고 완료 메시지 + 차감 수량
  - `showError(message)`

### 검증
- 수동 실행: CONFIRMED 주문 출고 처리 후 RELEASE 전환 확인
- 수동 실행: 출고 후 해당 시료 재고가 주문 수량만큼 차감됨 확인 (시료 목록 조회)
- 수동 실행: CONFIRMED 외 주문은 출고 목록에 미표시 확인
