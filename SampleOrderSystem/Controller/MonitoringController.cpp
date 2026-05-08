#include "MonitoringController.h"
#include "../Model/Domain/StockStatus.h"
#include <vector>
#include <unordered_map>
#include <cassert>

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

    // 유효 주문 수량 합계: RESERVED + PRODUCING + CONFIRMED 상태 주문의 quantity 합
    // (BR-03/BR-10 기반: RELEASE 시에만 재고 차감되므로 그 전 상태를 수요로 간주)
    // RELEASE(출고 완료)와 REJECTED(취소)는 제외

    // 1단계: sampleId별 유효 수량 합산 (O(M))
    std::unordered_map<std::string, int> qtyBySample;
    for (const auto& order : orders) {
        if (order.status == OrderStatus::RESERVED  ||
            order.status == OrderStatus::PRODUCING  ||
            order.status == OrderStatus::CONFIRMED) {
            qtyBySample[order.sampleId] += order.quantity;
        }
    }

    // 2단계: 시료별 상태 판단 (O(N))
    std::vector<StockStatus> statusList;
    statusList.reserve(samples.size());
    for (const auto& sample : samples) {
        int sum = qtyBySample.count(sample.id) ? qtyBySample.at(sample.id) : 0;
        statusList.push_back(evaluateStockStatus(sample.currentStock, sum));
    }

    assert(samples.size() == statusList.size()); // 불변식 보장
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
