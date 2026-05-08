#include "OrderView.h"
#include <iostream>
#include <iomanip>
#include <string>

void OrderView::showSubMenu()
{
    std::cout << "\n  [주문 관리]\n";
    std::cout << "  [1] 주문 접수\n";
    std::cout << "  [2] 승인·거절\n";
    std::cout << "  [0] 돌아가기\n";
    std::cout << "  선택: ";
}

int OrderView::getSubMenuChoice()
{
    int choice = 0;
    std::cin >> choice;
    if (choice < 0 || choice > 2) return 0;
    return choice;
}

std::tuple<std::string, std::string, int> OrderView::promptOrderInput()
{
    std::string sampleId;
    std::string customerName;
    int quantity = 0;

    std::cout << "  시료 ID: ";
    std::cin >> sampleId;
    std::cout << "  고객명: ";
    std::cin >> customerName;
    std::cout << "  수량: ";
    std::cin >> quantity;

    return { sampleId, customerName, quantity };
}

void OrderView::showReservedOrders(const std::vector<Order>& orders, const std::vector<Sample>& samples)
{
    if (orders.empty()) {
        std::cout << "  접수된 주문이 없습니다.\n";
        return;
    }

    std::cout << "\n  %-4s %-12s %-20s %-10s %-6s %-20s\n";
    std::cout << "  " << std::left
              << std::setw(4)  << "No."
              << std::setw(12) << "주문ID"
              << std::setw(20) << "시료명"
              << std::setw(10) << "고객명"
              << std::setw(6)  << "수량"
              << std::setw(20) << "접수일시"
              << "\n";
    std::cout << "  " << std::string(72, '-') << "\n";

    for (size_t i = 0; i < orders.size(); ++i) {
        const Order& o = orders[i];

        std::string sampleName = o.sampleId;
        for (const auto& s : samples) {
            if (s.id == o.sampleId) {
                sampleName = s.name;
                break;
            }
        }

        std::cout << "  " << std::left
                  << std::setw(4)  << (i + 1)
                  << std::setw(12) << o.id
                  << std::setw(20) << sampleName
                  << std::setw(10) << o.customerName
                  << std::setw(6)  << o.quantity
                  << std::setw(20) << o.createdAt
                  << "\n";
    }
}

int OrderView::promptOrderSelect(int maxIndex)
{
    int sel = 0;
    std::cout << "  처리할 번호 선택 (0=취소): ";
    std::cin >> sel;
    if (sel < 0 || sel > maxIndex) return 0;
    return sel;
}

void OrderView::showApprovalResult(OrderStatus newStatus)
{
    switch (newStatus) {
    case OrderStatus::CONFIRMED:
        std::cout << "  [완료] 주문이 승인되었습니다. (재고 충분 — CONFIRMED)\n";
        break;
    case OrderStatus::PRODUCING:
        std::cout << "  [완료] 주문이 승인되었습니다. 생산 대기 중 (PRODUCING)\n";
        break;
    case OrderStatus::REJECTED:
        std::cout << "  [완료] 주문이 거절되었습니다.\n";
        break;
    default:
        std::cout << "  [완료] 주문 상태가 변경되었습니다.\n";
        break;
    }
}

void OrderView::showError(const std::string& msg)
{
    std::cout << "  [오류] " << msg << "\n";
}

void OrderView::showSuccess(const std::string& msg)
{
    std::cout << "  [완료] " << msg << "\n";
}
