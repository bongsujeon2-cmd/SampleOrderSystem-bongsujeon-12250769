#pragma once
#include "ISubController.h"
#include "../Model/Repository/IOrderRepository.h"
#include "../Model/Repository/ISampleRepository.h"
#include "../View/IMonitoringView.h"

class MonitoringController : public ISubController {
public:
    MonitoringController(IOrderRepository& orderRepo,
                         ISampleRepository& sampleRepo,
                         IMonitoringView& view);
    void run() override;
    void showOrderStats();
    void showStockStatus();

private:
    IOrderRepository&  orderRepo_;
    ISampleRepository& sampleRepo_;
    IMonitoringView&   view_;
};
