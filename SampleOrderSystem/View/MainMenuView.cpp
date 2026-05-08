#include "MainMenuView.h"
#include <iostream>
#include <string>

void MainMenuView::showMainMenu(const std::vector<Sample>& samples)
{
    std::cout << "\n======= S-Semi 반도체 시료 생산주문관리 =======\n";
    std::cout << "[시료 재고 현황]\n";
    for (const auto& s : samples) {
        std::cout << "  " << s.id << " " << s.name
                  << "    재고: " << s.currentStock << "개\n";
    }
    std::cout << "[메뉴]\n";
    std::cout << "  [1] 시료 관리\n";
    std::cout << "  [2] 주문\n";
    std::cout << "  [3] 모니터링\n";
    std::cout << "  [4] 출고 처리\n";
    std::cout << "  [5] 생산 라인\n";
    std::cout << "  [0] 종료\n";
    std::cout << "선택: ";
}

int MainMenuView::getMenuChoice()
{
    int choice = -1;
    std::string line;
    if (std::getline(std::cin, line)) {
        try {
            choice = std::stoi(line);
        } catch (...) {
            choice = -1;
        }
    }
    return choice;
}

void MainMenuView::showInvalidInput()
{
    std::cout << "  잘못된 입력입니다. 0~5 사이 숫자를 입력하세요.\n";
}
