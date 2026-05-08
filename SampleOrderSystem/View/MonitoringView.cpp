#include "MonitoringView.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <cassert>

static void printOrderGroup(const std::string& label, const std::vector<Order>& orders) {
    std::cout << label << " (" << orders.size() << "건)";
    if (!orders.empty()) {
        std::cout << ": ";
        for (size_t i = 0; i < orders.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << orders[i].id;
        }
    }
    std::cout << "\n";
}

void MonitoringView::showSubMenu() {
    std::cout << "\n=== 모니터링 ===\n"
              << "  [1] 주문량 확인\n"
              << "  [2] 재고량 확인\n"
              << "  [0] 돌아가기\n"
              << "선택: ";
}

int MonitoringView::getSubMenuChoice() {
    int choice = -1;
    std::cin >> choice;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return -1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    if (choice < 0 || choice > 2) return -1;
    return choice;
}

void MonitoringView::showOrderStats(const std::vector<Order>& reserved,
                                    const std::vector<Order>& producing,
                                    const std::vector<Order>& confirmed,
                                    const std::vector<Order>& released) {
    std::cout << "\n=== 주문 현황 ===\n";

    printOrderGroup("  예약", reserved);
    printOrderGroup("  생산 중", producing);
    printOrderGroup("  확정", confirmed);
    printOrderGroup("  출고", released);
}

void MonitoringView::showStockStatus(const std::vector<Sample>& samples,
                                     const std::vector<StockStatus>& statusList) {
    assert(samples.size() == statusList.size());

    std::cout << "\n=== 재고 현황 ===\n"
              << std::left
              << std::setw(12) << "시료ID"
              << std::setw(20) << "시료명"
              << std::setw(8)  << "재고"
              << "상태\n"
              << std::string(50, '-') << "\n";

    for (size_t i = 0; i < samples.size(); ++i) {
        std::string statusStr;
        switch (statusList[i]) {
            case StockStatus::SURPLUS:  statusStr = "[여유]"; break;
            case StockStatus::SHORTAGE: statusStr = "[부족]"; break;
            case StockStatus::DEPLETED: statusStr = "[고갈]"; break;
        }
        std::cout << std::left
                  << std::setw(12) << samples[i].id
                  << std::setw(20) << samples[i].name
                  << std::setw(8)  << samples[i].currentStock
                  << statusStr << "\n";
    }
}
