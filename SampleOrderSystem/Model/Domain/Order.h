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

inline std::string toString(OrderStatus status) {
    switch (status) {
    case OrderStatus::RESERVED:  return "RESERVED";
    case OrderStatus::PRODUCING: return "PRODUCING";
    case OrderStatus::CONFIRMED: return "CONFIRMED";
    case OrderStatus::RELEASE:   return "RELEASE";
    case OrderStatus::REJECTED:  return "REJECTED";
    default:                     return "RESERVED";
    }
}

inline OrderStatus orderStatusFromString(const std::string& s) {
    if (s == "RESERVED")  return OrderStatus::RESERVED;
    if (s == "PRODUCING") return OrderStatus::PRODUCING;
    if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
    if (s == "RELEASE")   return OrderStatus::RELEASE;
    if (s == "REJECTED")  return OrderStatus::REJECTED;
    throw std::invalid_argument("Unknown OrderStatus: " + s);
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
};
