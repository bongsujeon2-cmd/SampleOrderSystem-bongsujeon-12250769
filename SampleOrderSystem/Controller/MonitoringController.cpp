#include "MonitoringController.h"
#include "../Model/Domain/StockStatus.h"
#include <vector>

MonitoringController::MonitoringController(IOrderRepository& orderRepo,
                                           ISampleRepository& sampleRepo,
                                           IMonitoringView& view)
    : orderRepo_(orderRepo)
    , sampleRepo_(sampleRepo)
    , view_(view)
{}

void MonitoringController::showOrderStats() {
    auto all = orderRepo_.findAll();

    std::vector<Order> reserved;
    std::vector<Order> producing;
    std::vector<Order> confirmed;
    std::vector<Order> released;

    for (const auto& o : all) {
        switch (o.status) {
            case OrderStatus::RESERVED:  reserved.push_back(o);  break;
            case OrderStatus::PRODUCING: producing.push_back(o); break;
            case OrderStatus::CONFIRMED: confirmed.push_back(o); break;
            case OrderStatus::RELEASE:   released.push_back(o);  break;
            case OrderStatus::REJECTED:  break;  // BR-12: 제외
        }
    }

    view_.showOrderStats(reserved, producing, confirmed, released);
}

void MonitoringController::showStockStatus() {
    auto samples = sampleRepo_.findAll();
    auto orders  = orderRepo_.findAll();

    std::vector<StockStatus> statusList;
    statusList.reserve(samples.size());

    for (const auto& sample : samples) {
        int validQtySum = 0;
        for (const auto& order : orders) {
            if (order.sampleId != sample.id) continue;
            if (order.status == OrderStatus::RESERVED  ||
                order.status == OrderStatus::PRODUCING  ||
                order.status == OrderStatus::CONFIRMED) {
                validQtySum += order.quantity;
            }
        }

        if (sample.currentStock == 0)
            statusList.push_back(StockStatus::DEPLETED);
        else if (sample.currentStock <= validQtySum)
            statusList.push_back(StockStatus::SHORTAGE);
        else
            statusList.push_back(StockStatus::SURPLUS);
    }

    view_.showStockStatus(samples, statusList);
}

void MonitoringController::run() {
    while (true) {
        view_.showSubMenu();
        int ch = view_.getSubMenuChoice();
        switch (ch) {
            case 1: showOrderStats();  break;
            case 2: showStockStatus(); break;
            case 0: return;
            default: break;
        }
    }
}
