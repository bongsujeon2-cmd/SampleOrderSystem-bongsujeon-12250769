#pragma once
#include "IOrderView.h"

class OrderView : public IOrderView {
public:
    void showSubMenu() override;
    int  getSubMenuChoice() override;
    OrderInput promptOrderInput() override;
    void showReservedOrders(const std::vector<Order>&, const std::vector<Sample>&) override;
    int  promptOrderSelect(int maxIndex) override;
    int  promptApproveOrReject() override;
    void showApprovalResult(OrderStatus newStatus) override;
    void showError(const std::string& msg) override;
    void showSuccess(const std::string& msg) override;
};
