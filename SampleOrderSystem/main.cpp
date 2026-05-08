// main.cpp — Composition Root
// 비즈니스 로직 없음. 인스턴스 생성 및 의존성 조립만 수행.

#include <gmock/gmock.h>
#include <windows.h>
#include <string>
#include <filesystem>

// Repository
#include "Model/Repository/JsonSampleRepository.h"
#include "Model/Repository/JsonOrderRepository.h"
#include "Model/Repository/JsonProductionRepository.h"

// Tools
#include "Tools/DataMonitorTool.h"

// Service
#include "Model/Service/RealTimeProvider.h"
#include "Model/Service/MockTimeProvider.h"
#include "Model/Service/ProductionService.h"

// View
#include "View/SampleView.h"
#include "View/OrderView.h"
#include "View/MonitoringView.h"
#include "View/ShipmentView.h"
#include "View/ProductionLineView.h"
#include "View/MainMenuView.h"

// Controller
#include "Controller/SampleController.h"
#include "Controller/OrderController.h"
#include "Controller/MonitoringController.h"
#include "Controller/ShipmentController.h"
#include "Controller/ProductionLineController.h"
#include "Controller/AppController.h"

namespace fs = std::filesystem;

static bool hasArg(int argc, char* argv[], const std::string& flag) {
    for (int i = 1; i < argc; ++i)
        if (std::string(argv[i]) == flag) return true;
    return false;
}

int main(int argc, char* argv[]) {
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    // --test: Google Test 실행
    if (hasArg(argc, argv, "--test")) {
        testing::InitGoogleTest(&argc, argv);
        return RUN_ALL_TESTS();
    }

    // data/ 디렉토리 보장 (공통)
    fs::create_directories("data");

    // --dump-data: JSON 데이터 파일 내용 출력
    if (hasArg(argc, argv, "--dump-data")) {
        JsonSampleRepository     sampleRepo("data/samples.json");
        JsonOrderRepository      orderRepo("data/orders.json");
        JsonProductionRepository productionRepo("data/production.json");
        return DataMonitorTool(sampleRepo, orderRepo, productionRepo).run();
    }

    // --mock-time 여부
    bool mockTime = hasArg(argc, argv, "--mock-time");

    // ── Repository ─────────────────────────────────────────────────────────
    JsonSampleRepository     sampleRepo("data/samples.json");
    JsonOrderRepository      orderRepo("data/orders.json");
    JsonProductionRepository productionRepo("data/production.json");

    // ── TimeProvider ────────────────────────────────────────────────────────
    RealTimeProvider  realTime;
    MockTimeProvider  mockTimeProv;
    ITimeProvider&    timeProvider = mockTime
                                     ? static_cast<ITimeProvider&>(mockTimeProv)
                                     : static_cast<ITimeProvider&>(realTime);

    // ── Service ─────────────────────────────────────────────────────────────
    ProductionService productionService(sampleRepo, orderRepo, productionRepo, timeProvider);

    // ── View ────────────────────────────────────────────────────────────────
    SampleView          sampleView;
    OrderView           orderView;
    MonitoringView      monitoringView;
    ShipmentView        shipmentView;
    ProductionLineView  productionLineView;
    MainMenuView        mainMenuView;

    // ── Controller ──────────────────────────────────────────────────────────
    SampleController        sampleCtrl(sampleRepo, sampleView);
    OrderController         orderCtrl(orderRepo, sampleRepo, productionRepo, timeProvider, orderView);
    MonitoringController    monitoringCtrl(orderRepo, sampleRepo, monitoringView);
    ShipmentController      shipmentCtrl(orderRepo, sampleRepo, shipmentView);
    ProductionLineController productionLineCtrl(productionService, productionRepo,
                                                sampleRepo, timeProvider,
                                                productionLineView, mockTime);
    AppController appCtrl(sampleCtrl, orderCtrl, monitoringCtrl, shipmentCtrl,
                          productionLineCtrl, productionService, sampleRepo, mainMenuView);

    appCtrl.run();
    return 0;
}
