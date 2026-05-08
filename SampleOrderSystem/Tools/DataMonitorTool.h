// PRD: "--dump-data 인수로 활성화, 세 JSON 파일 내용을 즉시 출력 후 종료"
// Design 문서의 인터랙티브 메뉴는 향후 확장 시 고려
#pragma once
#include <string>
#include "../Model/Repository/ISampleRepository.h"
#include "../Model/Repository/IOrderRepository.h"
#include "../Model/Repository/IProductionRepository.h"

class DataMonitorTool {
public:
    DataMonitorTool(ISampleRepository&, IOrderRepository&, IProductionRepository&);
    int run();
private:
    ISampleRepository&     sampleRepo_;
    IOrderRepository&      orderRepo_;
    IProductionRepository& productionRepo_;
    void showSamples();
    void showOrders();
    void showProduction();
    static void separator(const std::string& title);
};
