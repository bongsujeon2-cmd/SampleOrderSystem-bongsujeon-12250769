# PLAN: 모니터링 (Monitoring)

> **관련 문서**
> - PRD: [docs/prd/monitoring.md](../prd/monitoring.md)
> - Design: [docs/design/monitoring.md](../design/monitoring.md)

---

## 1. 데이터 조회 전략

모니터링은 **읽기 전용** 기능이다. 데이터 변경 없이 Repository를 읽어 집계한다.

**결정:** 별도 Service나 캐시 레이어 없이 `MonitoringController`가 Repository를 직접 조회.

**이유:**
- 집계 로직이 MonitoringController에서만 사용됨 (재사용 없음)
- 인메모리 필터링으로 충분 (시료·주문 수 수십~수백 개 수준)
- Repository 싱글턴이 인메모리 상태를 최신으로 유지하므로 파일 재읽기 불필요

---

## 2. 주문량 확인 (BR-12)

```
MonitoringController::showOrderStats():
  orders = IOrderRepository::findAll()

  reserved  = filter(status == RESERVED)
  producing = filter(status == PRODUCING)
  confirmed = filter(status == CONFIRMED)
  released  = filter(status == RELEASE)
  // REJECTED 제외 (BR-12)

  View: 각 상태별 건수 + 목록 표시
```

---

## 3. 재고량 확인

### 3.1 유효 주문 수량 합계 정의 (PRD)

유효 주문 수량 합계 = 해당 시료에 대해 **RESERVED + PRODUCING + CONFIRMED** 상태인 주문의 quantity 합계.

- **RELEASE 제외:** 출고 완료로 이미 재고 차감됨
- **REJECTED 제외:** 취소된 주문

### 3.2 재고 상태 판단 로직

```
MonitoringController::showStockStatus():
  samples = ISampleRepository::findAll()
  orders  = IOrderRepository::findAll()

  for each sample:
    activeOrders = filter(orders,
        sampleId == sample.id
        AND status in {RESERVED, PRODUCING, CONFIRMED})
    validQtySum = sum(activeOrders.quantity)

    if sample.currentStock == 0:
        status = "[고갈]"
    elif sample.currentStock <= validQtySum:
        status = "[부족]"
    else:
        status = "[여유]"

    View: 시료 ID, 이름, 현재 재고, status 출력
```

### 3.3 엣지 케이스

| 케이스 | 처리 |
|--------|------|
| 유효 주문이 없고 재고 > 0 | `validQtySum = 0` → currentStock > 0 → **[여유]** |
| 유효 주문이 없고 재고 = 0 | currentStock == 0 → **[고갈]** |

---

## 4. 최신 데이터 보장

Repository 싱글턴이 인메모리 상태를 실시간으로 반영하므로, 매 조회 시 `findAll()`을 호출하면 항상 최신 데이터를 얻는다 (main-menu PLAN의 싱글턴 전략 참조).

별도 "새로고침" 메커니즘 불필요.
