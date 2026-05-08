#pragma once
#include "../Model/Domain/Order.h"
#include "../Model/Domain/Sample.h"
#include "../Model/Domain/StockStatus.h"
#include <vector>

class IMonitoringView {
public:
    virtual ~IMonitoringView() = default;
    virtual void showSubMenu() = 0;
    virtual int  getSubMenuChoice() = 0;
    virtual void showOrderStats(const std::vector<Order>& reserved,
                                const std::vector<Order>& producing,
                                const std::vector<Order>& confirmed,
                                const std::vector<Order>& released) = 0;
    virtual void showStockStatus(const std::vector<Sample>& samples,
                                 const std::vector<StockStatus>& statusList) = 0;
};
