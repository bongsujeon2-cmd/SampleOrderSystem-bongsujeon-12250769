#pragma once

enum class StockStatus { SURPLUS, SHORTAGE, DEPLETED };

// BR-03/BR-10: 재고 상태 판단 유틸
// currentStock이 0이면 DEPLETED, 유효 주문 합계 이하면 SHORTAGE, 초과면 SURPLUS
inline StockStatus evaluateStockStatus(int currentStock, int validQtySum) {
    if (currentStock == 0)           return StockStatus::DEPLETED;
    if (currentStock <= validQtySum) return StockStatus::SHORTAGE;
    return StockStatus::SURPLUS;
}
