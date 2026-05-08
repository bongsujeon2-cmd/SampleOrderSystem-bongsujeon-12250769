#pragma once
#include "../Model/Repository/IOrderRepository.h"
#include "../Model/Repository/ISampleRepository.h"
#include "../View/IMonitoringView.h"

class MonitoringController {
public:
    MonitoringController(IOrderRepository& orderRepo,
                         ISampleRepository& sampleRepo,
                         IMonitoringView& view);
    void run();
    void showOrderStats();
    void showStockStatus();

private:
    IOrderRepository&  orderRepo_;
    ISampleRepository& sampleRepo_;
    IMonitoringView&   view_;
};
