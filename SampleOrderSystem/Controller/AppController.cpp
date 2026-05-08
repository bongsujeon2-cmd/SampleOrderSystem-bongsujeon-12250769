#include "AppController.h"

AppController::AppController(ISubController& sampleCtrl,
                             ISubController& orderCtrl,
                             ISubController& monitoringCtrl,
                             ISubController& shipmentCtrl,
                             ISubController& productionLineCtrl,
                             IProductionService& productionService,
                             ISampleRepository& sampleRepo,
                             IMainMenuView& view)
    : sampleCtrl_(sampleCtrl)
    , orderCtrl_(orderCtrl)
    , monitoringCtrl_(monitoringCtrl)
    , shipmentCtrl_(shipmentCtrl)
    , productionLineCtrl_(productionLineCtrl)
    , productionService_(productionService)
    , sampleRepo_(sampleRepo)
    , view_(view)
{
}

void AppController::run()
{
    while (true) {
        productionService_.checkAndComplete();
        auto samples = sampleRepo_.findAll();
        view_.showMainMenu(samples);
        int choice = view_.getMenuChoice();
        switch (choice) {
            case 1: sampleCtrl_.run();           break;
            case 2: orderCtrl_.run();            break;
            case 3: monitoringCtrl_.run();       break;
            case 4: shipmentCtrl_.run();         break;
            case 5: productionLineCtrl_.run();   break;
            case 0: return;
            default: view_.showInvalidInput();   break;
        }
    }
}
