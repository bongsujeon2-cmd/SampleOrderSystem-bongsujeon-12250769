#include "SampleView.h"
#include <iostream>
#include <iomanip>
#include <limits>
#include <string>

void SampleView::showSubMenu() {
    std::cout << "\n=== 시료 관리 ===\n"
              << "  [1] 시료 등록\n"
              << "  [2] 목록 조회\n"
              << "  [3] 이름 검색\n"
              << "  [0] 돌아가기\n"
              << "선택: ";
}

int SampleView::getSubMenuChoice() {
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

Sample SampleView::promptSampleInput() {
    Sample s;
    std::cout << "  ID: ";
    std::getline(std::cin, s.id);
    std::cout << "  이름: ";
    std::getline(std::cin, s.name);
    std::cout << "  평균 생산시간(분): ";
    std::cin >> s.avgProductionTime;
    std::cout << "  수율(0.0~1.0): ";
    std::cin >> s.yieldRate;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    s.currentStock = 0;
    return s;
}

static void printTableHeader() {
    std::cout << "\n"
              << std::left
              << std::setw(10) << "ID"
              << std::setw(20) << "이름"
              << std::setw(12) << "수율"
              << std::setw(18) << "평균생산시간(분)"
              << std::setw(8)  << "재고"
              << "\n"
              << std::string(68, '-') << "\n";
}

static void printTableRow(const Sample& s) {
    std::cout << std::left
              << std::setw(10) << s.id
              << std::setw(20) << s.name
              << std::setw(12) << s.yieldRate
              << std::setw(18) << s.avgProductionTime
              << std::setw(8)  << s.currentStock
              << "\n";
}

static void showSampleTable(const std::vector<Sample>& samples) {
    printTableHeader();
    for (const auto& s : samples)
        printTableRow(s);
}

void SampleView::showSampleList(const std::vector<Sample>& samples) {
    showSampleTable(samples);
}

void SampleView::showSearchResult(const std::vector<Sample>& results) {
    showSampleTable(results);
}

void SampleView::showError(const std::string& msg) {
    std::cout << "  [오류] " << msg << "\n";
}

void SampleView::showSuccess(const std::string& msg) {
    std::cout << "  [완료] " << msg << "\n";
}

std::string SampleView::promptKeyword() {
    std::string keyword;
    std::cout << "  검색어: ";
    std::getline(std::cin, keyword);
    return keyword;
}
