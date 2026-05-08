# Design: 주문 접수 및 승인/거절 (Order)

> **관련 문서**
> - PRD: [docs/prd/order.md](../prd/order.md)
> - PLAN: [docs/plan/order.md](../plan/order.md)

> **선행 조건:** `sample-management` Design의 Phase 1~3 완료 (기반 인프라 + ISampleRepository 구현)

---

## Phase 1: JsonOrderRepository 구현

### Tasks

#### 테스트 코드
- [ ] `test/OrderRepositoryTest.cpp` 작성
  - `create()` 후 반환 ID가 `"ORD-001"` 형식인지 확인
  - `create()` 두 번 호출 시 `"ORD-001"`, `"ORD-002"` 순차 생성 확인
  - 앱 재시작 시뮬레이션: Repository 재생성 후 `nextOrdNum` 이어서 증가 확인
  - `findByStatus(RESERVED)`: 해당 상태 주문만 반환 확인
  - `update()`: 상태 변경 후 `findById()`로 반영 확인
  - `findAll()`: 전체 주문 반환 확인

#### 구현
- [ ] `Model/Repository/JsonOrderRepository.h/.cpp`
  - 생성자에서 `data/orders.json` 로드 (`nextOrdNum` + `orders` 배열)
  - `create()`: `nextOrdNum`으로 `"ORD-NNN"` 생성 → 저장 → 파일 flush
  - `findById()`, `findAll()`, `findByStatus()`: 인메모리 조회
  - `update()`: 인메모리 수정 + 파일 flush

### 검증
- 모든 테스트 PASS
- `data/orders.json` 파일이 PRD 스키마 형식과 일치하는지 육안 확인

---

## Phase 2: OrderController 구현

### Tasks

#### 테스트 코드
- [ ] `test/OrderControllerTest.cpp` 작성 (Mock 레포지토리 사용)

  **주문 접수**
  - `placeOrder()`: 존재하는 sampleId → `IOrderRepository::create()` 호출 확인
  - `placeOrder()`: 미등록 sampleId → 에러 반환, `create()` 미호출 확인
  - `placeOrder()`: 수량 0 이하 → 에러 반환

  **주문 승인 — 재고 충분**
  - 현재 재고 >= 주문 수량 → `order.status = CONFIRMED`, `IOrderRepository::update()` 호출 확인
  - 재고 차감 없음: `ISampleRepository::update()` 미호출 확인

  **주문 승인 — 재고 부족**
  - 현재 재고 < 주문 수량 → `order.status = PRODUCING` 확인
  - `IProductionRepository::enqueue()` 호출 확인
  - 생산 수량 계산 검증: 부족분=10, 수율=0.9 → `actualProductionQty = ceil(10/(0.9×0.9)) = 13`
  - 총 생산 시간: `avgProductionTime × actualProductionQty` 확인

  **주문 거절**
  - `rejectOrder()`: `order.status = REJECTED`, `update()` 호출 확인

#### 구현
- [ ] `Controller/OrderController.h/.cpp`
  - `placeOrder()`: sampleId 존재 확인 → `IOrderRepository::create()` (status=RESERVED, createdAt=`ITimeProvider::nowIso8601()`)
  - `listReservedOrders()`: `findByStatus(RESERVED)` → View 렌더링
  - `approveOrder(orderId)`: 재고 분기 → CONFIRMED 또는 PRODUCING + 생산 수량 계산 + enqueue
  - `rejectOrder(orderId)`: status → REJECTED + update

### 검증
- 모든 테스트 PASS

---

## Phase 3: OrderView 구현

### Tasks

- [ ] `View/OrderView.h/.cpp`
  - `showSubMenu()`: [1] 주문 접수 / [2] 승인·거절 / [0] 돌아가기
  - `promptOrderInput()`: sampleId / 고객명 / 수량 입력
  - `showReservedOrders(orders, samples)`: 주문 ID / 시료명 / 고객명 / 수량 / 접수일시 테이블
  - `promptOrderSelect(orders)`: 처리할 주문 번호 선택
  - `showApprovalResult(status)`: CONFIRMED 또는 PRODUCING 결과 메시지
  - `showError(message)`, `showSuccess(message)`

### 검증
- 수동 실행: 주문 접수 후 RESERVED 상태로 `orders.json` 저장 확인
- 수동 실행: 재고 충분 승인 → CONFIRMED 전환 확인
- 수동 실행: 재고 부족 승인 → PRODUCING 전환 + `production.json` queue 등록 확인
- 수동 실행: 미등록 sampleId 입력 시 에러 메시지 출력 확인
- 수동 실행: 거절 후 모니터링 조회에서 해당 주문 미포함 확인
