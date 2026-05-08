#pragma once
#include "../Domain/Order.h"
#include <optional>
#include <vector>
#include <string>

class IOrderRepository {
public:
    virtual ~IOrderRepository() = default;
    virtual std::string          create(const Order& order) = 0;
    virtual std::optional<Order> findById(const std::string& id) const = 0;
    virtual std::vector<Order>   findAll() const = 0;
    virtual std::vector<Order>   findByStatus(OrderStatus status) const = 0;
    virtual bool                 update(const Order& order) = 0;
    virtual void                 clearAll() = 0;
};
