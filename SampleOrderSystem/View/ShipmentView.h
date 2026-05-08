#pragma once
#include "IShipmentView.h"

class ShipmentView : public IShipmentView {
public:
    void        showSubMenu() override;
    int         getSubMenuChoice() override;
    void        showConfirmedOrders(const std::vector<Order>& orders,
                                    const std::vector<Sample>& samples) override;
    void        showNoConfirmedOrders() override;
    std::string promptOrderIdInput() override;
    void        showShipmentSuccess(const std::string& orderId, int qty) override;
    void        showError(const std::string& msg) override;
};
