#pragma once
#include "../Model/Repository/JsonSampleRepository.h"
#include "../Model/Repository/JsonOrderRepository.h"
#include "../Model/Repository/JsonProductionRepository.h"

class DataMonitorTool {
public:
    DataMonitorTool(const std::string& dataDir = "data");
    int run();
private:
    std::string dataDir_;
    void showSamples();
    void showOrders();
    void showProduction();
    static void separator(const std::string& title);
};
