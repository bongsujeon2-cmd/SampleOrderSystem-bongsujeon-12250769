#include "DataMonitorTool.h"
#include <iostream>
#include <iomanip>
#include <ctime>

DataMonitorTool::DataMonitorTool(const std::string& dataDir)
    : dataDir_(dataDir)
{
}

int DataMonitorTool::run() {
    separator(dataDir_ + "/samples.json");
    showSamples();

    separator(dataDir_ + "/orders.json");
    showOrders();

    separator(dataDir_ + "/production.json");
    showProduction();

    std::cout << "\n";
    return 0;
}

void DataMonitorTool::showSamples() {
    JsonSampleRepository repo(dataDir_ + "/samples.json");
    auto samples = repo.findAll();

    std::cout << "  "
              << std::left
              << std::setw(12) << "ID"
              << std::setw(20) << "이름"
              << std::setw(20) << "평균생산시간(분)"
              << std::setw(10) << "수율"
              << std::setw(10) << "재고"
              << "\n";
    std::cout << "  " << std::string(70, '-') << "\n";

    if (samples.empty()) {
        std::cout << "  (데이터 없음)\n";
        return;
    }

    for (const auto& s : samples) {
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
    JsonOrderRepository repo(dataDir_ + "/orders.json");
    auto orders = repo.findAll();

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

    if (orders.empty()) {
        std::cout << "  (데이터 없음)\n";
        return;
    }

    auto statusToStr = [](OrderStatus st) -> std::string {
        switch (st) {
        case OrderStatus::RESERVED:  return "RESERVED";
        case OrderStatus::PRODUCING: return "PRODUCING";
        case OrderStatus::CONFIRMED: return "CONFIRMED";
        case OrderStatus::RELEASE:   return "RELEASE";
        case OrderStatus::REJECTED:  return "REJECTED";
        default:                     return "UNKNOWN";
        }
    };

    for (const auto& o : orders) {
        std::cout << "  "
                  << std::left
                  << std::setw(12) << o.id
                  << std::setw(12) << o.sampleId
                  << std::setw(16) << o.customerName
                  << std::setw(8)  << o.quantity
                  << std::setw(12) << statusToStr(o.status)
                  << std::setw(24) << o.createdAt
                  << "\n";
    }
}

void DataMonitorTool::showProduction() {
    JsonProductionRepository repo(dataDir_ + "/production.json");
    const auto& state = repo.getState();

    // activeJob
    if (!state.activeJob.has_value()) {
        std::cout << "  activeJob: (없음)\n";
    } else {
        const auto& job = state.activeJob.value();
        std::cout << "  activeJob:\n";
        std::cout << "    orderId             : " << job.orderId << "\n";
        std::cout << "    sampleId            : " << job.sampleId << "\n";
        std::cout << "    shortage            : " << job.shortage << "\n";
        std::cout << "    actualProductionQty : " << job.actualProductionQty << "\n";
        std::cout << "    totalProductionTime : " << job.totalProductionTimeMin << " 분\n";
        std::cout << "    startTimeUnix       : " << job.startTimeUnix << "\n";
    }

    // queue
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
