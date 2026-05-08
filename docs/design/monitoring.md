# Design: 모니터링 (Monitoring)

> **관련 문서**
> - PRD: [docs/prd/monitoring.md](../prd/monitoring.md)
> - PLAN: [docs/plan/monitoring.md](../plan/monitoring.md)

> **선행 조건:** `sample-management` + `order` Design의 모든 Phase 완료

---

## Phase 1: MonitoringController 구현

### Tasks

#### 테스트 코드
- [ ] `test/MonitoringControllerTest.cpp` 작성 (Mock 레포지토리 사용)

  **주문량 집계**
  - RESERVED 2건 / PRODUCING 1건 / CONFIRMED 3건 / RELEASE 1건 / REJECTED 2건 데이터 세팅
  - `showOrderStats()`: REJECTED 2건 제외 후 각 상태 건수 정확히 집계 확인 (BR-12)

  **재고 상태 — 여유**
  - `sample.currentStock = 100`, 유효 주문 합계 = 30 → `[여유]` 반환 확인

  **재고 상태 — 부족**
  - `sample.currentStock = 30`, 유효 주문 합계 = 30 → `[부족]` 반환 확인
  - `sample.currentStock = 20`, 유효 주문 합계 = 30 → `[부족]` 반환 확인

  **재고 상태 — 고갈**
  - `sample.currentStock = 0` → `[고갈]` 반환 확인 (유효 주문 합계 무관)

  **유효 주문 수량 합계 필터링**
  - RELEASE 주문: 유효 합계에서 제외 확인
  - REJECTED 주문: 유효 합계에서 제외 확인
  - RESERVED / PRODUCING / CONFIRMED: 유효 합계에 포함 확인

  **유효 주문 없음 엣지 케이스**
  - 시료에 대한 유효 주문 없음 + `currentStock > 0` → `[여유]` 확인

#### 구현
- [ ] `Controller/MonitoringController.h/.cpp`
  - `showOrderStats()`:
    - `IOrderRepository::findAll()` 전체 조회
    - status 기준 그룹핑 (REJECTED 제외)
    - 상태별 건수 + 주문 목록 → View 렌더링
  - `showStockStatus()`:
    - `ISampleRepository::findAll()` + `IOrderRepository::findAll()`
    - 각 시료별 유효 주문 수량 합계 계산 (RESERVED + PRODUCING + CONFIRMED만)
    - `currentStock == 0` → 고갈 / `<= 합계` → 부족 / `> 합계` → 여유 판단
    - → View 렌더링

### 검증
- 모든 테스트 PASS

---

## Phase 2: MonitoringView 구현

### Tasks

- [ ] `View/MonitoringView.h/.cpp`
  - `showSubMenu()`: [1] 주문량 확인 / [2] 재고량 확인 / [0] 돌아가기
  - `showOrderStats(reserved, producing, confirmed, released)`: 상태별 건수 + 주문 목록 출력
  - `showStockStatus(samples, stockStatusList)`: 시료별 재고 / 상태 테이블 출력

### 검증
- 수동 실행: REJECTED 주문이 집계에서 제외되는지 확인
- 수동 실행: 출고 처리 후 시료 재고 감소 → 재고 상태 변화 (여유→부족 등) 확인
- 수동 실행: 재고 = 0 시료 → `[고갈]` 표시 확인
