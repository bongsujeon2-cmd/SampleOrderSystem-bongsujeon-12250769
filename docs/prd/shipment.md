# PRD: 출고 처리 (Shipment)

> **관련 문서**
> - PLAN: [docs/plan/shipment.md](../../plan/shipment.md)
> - Design: [docs/design/shipment.md](../../design/shipment.md)

---

## 배경 및 목적

출고 대기 상태(CONFIRMED)인 주문을 실제 출고 처리하여 재고를 차감하고 주문을 완료 상태로 전환한다.

---

## 사용자 스토리

- 담당자로서, 출고 대기 중인 주문 목록을 확인하고 원하는 주문을 선택해 출고 처리하고 싶다.
- 담당자로서, 출고 처리 시 재고가 자동으로 차감되어 현황이 정확히 반영되길 원한다.

---

## 기능 요구사항

- CONFIRMED 상태 주문 목록 표시
- 특정 주문 선택 후 출고 실행
- 출고 처리 시:
  1. 해당 주문 수량만큼 재고 차감
  2. 주문 상태: CONFIRMED → RELEASE

---

## 비기능 요구사항

- 재고는 오직 RELEASE 출고 처리 시점에만 차감된다 (CONFIRMED 전환 시 차감 없음)
- CONFIRMED 상태 주문만 출고 처리 가능하다

---

## 성공 기준

- 출고 처리 후 주문 상태가 RELEASE로 전환된다
- 출고 처리 후 해당 시료의 재고가 주문 수량만큼 차감된다
- CONFIRMED가 아닌 주문은 출고 처리 목록에 표시되지 않는다
