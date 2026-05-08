#pragma once
#include "../Model/Domain/Order.h"
#include "../Model/Domain/Sample.h"
#include <vector>
#include <string>
#include <tuple>

class IOrderView {
public:
    virtual ~IOrderView() = default;
    virtual void showSubMenu() = 0;
    virtual int  getSubMenuChoice() = 0;
    virtual std::tuple<std::string, std::string, int> promptOrderInput() = 0;
    virtual void showReservedOrders(const std::vector<Order>&, const std::vector<Sample>&) = 0;
    virtual int  promptOrderSelect(int maxIndex) = 0;
    virtual void showApprovalResult(OrderStatus newStatus) = 0;
    virtual void showError(const std::string& msg) = 0;
    virtual void showSuccess(const std::string& msg) = 0;
};
