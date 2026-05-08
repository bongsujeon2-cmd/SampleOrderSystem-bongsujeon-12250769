#pragma once
#include "../Model/Domain/Order.h"
#include "../Model/Domain/Sample.h"
#include <vector>
#include <string>

class IShipmentView {
public:
    virtual ~IShipmentView() = default;
    virtual void        showSubMenu() = 0;
    virtual int         getSubMenuChoice() = 0;
    virtual void        showConfirmedOrders(const std::vector<Order>& orders,
                                            const std::vector<Sample>& samples) = 0;
    virtual void        showNoConfirmedOrders() = 0;
    virtual std::string promptOrderIdInput() = 0;
    virtual void        showShipmentSuccess(const std::string& orderId, int qty) = 0;
    virtual void        showError(const std::string& msg) = 0;
};
