#pragma once
#include <string>
#include <stdexcept>

enum class OrderStatus {
    RESERVED,
    PRODUCING,
    CONFIRMED,
    RELEASE,
    REJECTED
};

inline std::string toString(OrderStatus s) {
    switch (s) {
    case OrderStatus::RESERVED:  return "RESERVED";
    case OrderStatus::PRODUCING: return "PRODUCING";
    case OrderStatus::CONFIRMED: return "CONFIRMED";
    case OrderStatus::RELEASE:   return "RELEASE";
    case OrderStatus::REJECTED:  return "REJECTED";
    default:                     return "UNKNOWN";
    }
}

struct Order {
    std::string id;
    std::string sampleId;
    std::string customerName;
    int         quantity  = 0;
    OrderStatus status    = OrderStatus::RESERVED;
    std::string createdAt;

    bool operator==(const Order& other) const {
        return id           == other.id
            && sampleId     == other.sampleId
            && customerName == other.customerName
            && quantity     == other.quantity
            && status       == other.status
            && createdAt    == other.createdAt;
    }

    bool operator!=(const Order& other) const {
        return !(*this == other);
    }

    /// 도메인 유효성 검사. 오류 메시지 반환; 유효하면 빈 문자열.
    std::string validate() const {
        if (quantity <= 0)      return "주문 수량은 1 이상이어야 합니다.";
        if (sampleId.empty())   return "시료 ID가 비어 있습니다.";
        if (customerName.empty()) return "고객명이 비어 있습니다.";
        return "";
    }
};
