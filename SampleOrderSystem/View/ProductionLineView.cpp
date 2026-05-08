#include "ProductionLineView.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <limits>

void ProductionLineView::showSubMenu(bool isMockMode)
{
    std::cout << "\n===== [생산 라인 관리] =====\n";
    std::cout << "  [1] 생산 현황\n";
    std::cout << "  [2] 대기 큐\n";
    if (isMockMode) {
        std::cout << "  [3] 시간 앞당기기\n";
    }
    std::cout << "  [0] 돌아가기\n";
    std::cout << "선택: ";
}

int ProductionLineView::getSubMenuChoice()
{
    int ch = 0;
    std::cin >> ch;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        ch = -1;
    }
    return ch;
}

void ProductionLineView::showActiveJob(
    const ProductionJob& job,
    const std::string& sampleName,
    int elapsedMinutes,
    std::string expectedFinish)
{
    std::cout << "\n--- 현재 생산 중인 작업 ---\n";
    std::cout << "  주문 ID    : " << job.orderId << "\n";
    std::cout << "  시료명     : " << sampleName << "\n";
    std::cout << "  실생산량   : " << job.actualProductionQty << "\n";
    std::cout << "  총 소요시간: " << job.totalProductionTimeMin << " 분\n";
    std::cout << "  경과시간   : " << elapsedMinutes << " 분\n";
    std::cout << "  예상완료   : " << expectedFinish << "\n";
}

void ProductionLineView::showNoActiveJob()
{
    std::cout << "\n  현재 생산 중인 작업이 없습니다.\n";
}

void ProductionLineView::showQueue(
    const std::vector<ProductionJob>& queue,
    const std::vector<std::string>& sampleNames)
{
    std::cout << "\n--- 생산 대기 큐 ---\n";
    if (queue.empty()) {
        std::cout << "  대기 중인 작업이 없습니다.\n";
        return;
    }
    std::cout << std::left
              << std::setw(6) << "순번"
              << std::setw(12) << "주문 ID"
              << std::setw(16) << "시료명"
              << std::setw(10) << "실생산량"
              << std::setw(12) << "예상소요(분)"
              << "\n";
    for (size_t i = 0; i < queue.size(); ++i) {
        const auto& job = queue[i];
        const auto& name = (i < sampleNames.size()) ? sampleNames[i] : "";
        std::cout << std::left
                  << std::setw(6) << (i + 1)
                  << std::setw(12) << job.orderId
                  << std::setw(16) << name
                  << std::setw(10) << job.actualProductionQty
                  << std::setw(12) << job.totalProductionTimeMin
                  << "\n";
    }
}

int ProductionLineView::promptAdvanceMinutes()
{
    std::cout << "앞당길 분을 입력하세요: ";
    int min = 0;
    std::cin >> min;
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        min = 0;
    }
    return min;
}

void ProductionLineView::showError(const std::string& msg)
{
    std::cout << "  [오류] " << msg << "\n";
}
