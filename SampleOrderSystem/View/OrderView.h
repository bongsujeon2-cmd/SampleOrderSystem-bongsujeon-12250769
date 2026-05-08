#pragma once
#include "IOrderView.h"

class OrderView : public IOrderView {
public:
    void showSubMenu() override;
    int  getSubMenuChoice() override;
    std::tuple<std::string, std::string, int> promptOrderInput() override;
    void showReservedOrders(const std::vector<Order>&, const std::vector<Sample>&) override;
    int  promptOrderSelect(int maxIndex) override;
    void showApprovalResult(OrderStatus newStatus) override;
    void showError(const std::string& msg) override;
    void showSuccess(const std::string& msg) override;
};
