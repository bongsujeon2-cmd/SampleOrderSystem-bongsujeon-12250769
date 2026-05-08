#include "DataMonitorTool.h"
#include <iostream>
#include <iomanip>

DataMonitorTool::DataMonitorTool(ISampleRepository& sampleRepo,
                                 IOrderRepository& orderRepo,
                                 IProductionRepository& productionRepo)
    : sampleRepo_(sampleRepo)
    , orderRepo_(orderRepo)
    , productionRepo_(productionRepo)
{
}

int DataMonitorTool::run() {
    separator("samples.json");
    showSamples();

    separator("orders.json");
    showOrders();

    separator("production.json");
    showProduction();

    std::cout << "\n";
    return 0;
}

void DataMonitorTool::showSamples() {
    auto all = sampleRepo_.findAll();
    if (all.empty()) {
        std::cout << "  (데이터 없음)\n";
        return;
    }

    std::cout << "  "
              << std::left
              << std::setw(12) << "ID"
              << std::setw(20) << "이름"
              << std::setw(20) << "평균생산시간(분)"
              << std::setw(10) << "수율"
              << std::setw(10) << "재고"
              << "\n";
    std::cout << "  " << std::string(70, '-') << "\n";

    for (const auto& s : all) {
        std::cout << "  "
                  << std::left
                  << std::setw(12) << s.id
                  << std::setw(20) << s.name
                  << std::setw(20) << s.avgProductionTime
                  << std::setw(10) << s.yieldRate
                  << std::setw(10) << s.currentStock
                  << "\n";
    }
}

void DataMonitorTool::showOrders() {
    auto all = orderRepo_.findAll();
    if (all.empty()) {
        std::cout << "  (데이터 없음)\n";
        return;
    }

    std::cout << "  "
              << std::left
              << std::setw(12) << "주문ID"
              << std::setw(12) << "시료ID"
              << std::setw(16) << "고객명"
              << std::setw(8)  << "수량"
              << std::setw(12) << "상태"
              << std::setw(24) << "접수일시"
              << "\n";
    std::cout << "  " << std::string(82, '-') << "\n";

    for (const auto& o : all) {
        std::cout << "  "
                  << std::left
                  << std::setw(12) << o.id
                  << std::setw(12) << o.sampleId
                  << std::setw(16) << o.customerName
                  << std::setw(8)  << o.quantity
                  << std::setw(12) << toString(o.status)
                  << std::setw(24) << o.createdAt
                  << "\n";
    }
}

void DataMonitorTool::showProduction() {
    const auto& state = productionRepo_.getState();

    if (!state.activeJob.has_value()) {
        std::cout << "  activeJob: (없음)\n";
    } else {
        const auto& job = state.activeJob.value();
        std::cout << "  activeJob:\n";
        std::cout << "    orderId                : " << job.orderId << "\n";
        std::cout << "    sampleId               : " << job.sampleId << "\n";
        std::cout << "    shortage               : " << job.shortage << "\n";
        std::cout << "    actualProductionQty    : " << job.actualProductionQty << "\n";
        std::cout << "    totalProductionTimeMin : " << job.totalProductionTimeMin << " 분\n";
        std::cout << "    startTimeUnix          : " << job.startTimeUnix << "\n";
    }

    std::cout << "  queue (" << state.queue.size() << "건):\n";
    if (state.queue.empty()) {
        std::cout << "    (대기 없음)\n";
    } else {
        int idx = 1;
        for (const auto& job : state.queue) {
            std::cout << "    [" << idx++ << "] "
                      << "orderId=" << job.orderId
                      << "  sampleId=" << job.sampleId
                      << "  shortage=" << job.shortage
                      << "  actualQty=" << job.actualProductionQty
                      << "  time=" << job.totalProductionTimeMin << "분"
                      << "\n";
        }
    }
}

void DataMonitorTool::separator(const std::string& title) {
    std::cout << "\n============================================================\n";
    std::cout << "  " << title << "\n";
    std::cout << "============================================================\n";
}
