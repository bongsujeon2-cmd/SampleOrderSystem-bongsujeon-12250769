#include "ShipmentView.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <algorithm>

void ShipmentView::showSubMenu() {
    std::cout << "\n=== 출고 관리 ===\n"
              << "  [1] 출고 대기 목록\n"
              << "  [2] 출고 처리\n"
              << "  [0] 돌아가기\n"
              << "선택: ";
}

int ShipmentView::getSubMenuChoice() {
    int choice = -1;
    std::cin >> choice;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return -1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return choice;
}

void ShipmentView::showConfirmedOrders(const std::vector<Order>& orders,
                                       const std::vector<Sample>& samples) {
    std::cout << "\n"
              << std::left
              << std::setw(12) << "주문ID"
              << std::setw(20) << "시료명"
              << std::setw(20) << "고객명"
              << std::setw(8)  << "수량"
              << "\n"
              << std::string(60, '-') << "\n";

    for (const auto& order : orders) {
        std::string sampleName = "-";
        auto it = std::find_if(samples.begin(), samples.end(),
            [&](const Sample& s) { return s.id == order.sampleId; });
        if (it != samples.end())
            sampleName = it->name;

        std::cout << std::left
                  << std::setw(12) << order.id
                  << std::setw(20) << sampleName
                  << std::setw(20) << order.customerName
                  << std::setw(8)  << order.quantity
                  << "\n";
    }
}

void ShipmentView::showNoConfirmedOrders() {
    std::cout << "  출고 대기 주문이 없습니다.\n";
}

std::string ShipmentView::promptOrderIdInput() {
    std::string id;
    std::cout << "  출고할 주문 ID: ";
    std::getline(std::cin, id);
    return id;
}

void ShipmentView::showShipmentSuccess(const std::string& orderId, int qty) {
    std::cout << "  [완료] 주문 " << orderId
              << " 출고 처리 완료 (" << qty << "개 재고 차감)\n";
}

void ShipmentView::showError(const std::string& msg) {
    std::cout << "  [오류] " << msg << "\n";
}
