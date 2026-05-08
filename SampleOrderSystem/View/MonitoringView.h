#pragma once
#include "IMonitoringView.h"

class MonitoringView : public IMonitoringView {
public:
    void showSubMenu() override;
    int  getSubMenuChoice() override;
    void showOrderStats(const std::vector<Order>& reserved,
                        const std::vector<Order>& producing,
                        const std::vector<Order>& confirmed,
                        const std::vector<Order>& released) override;
    void showStockStatus(const std::vector<Sample>& samples,
                         const std::vector<StockStatus>& statusList) override;
};
