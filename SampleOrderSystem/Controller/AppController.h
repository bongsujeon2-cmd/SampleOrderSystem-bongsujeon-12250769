#pragma once
#include "ISubController.h"
#include "../Model/Service/IProductionService.h"
#include "../Model/Repository/ISampleRepository.h"
#include "../View/IMainMenuView.h"

class AppController {
public:
    AppController(ISubController& sampleCtrl,
                  ISubController& orderCtrl,
                  ISubController& monitoringCtrl,
                  ISubController& shipmentCtrl,
                  ISubController& productionLineCtrl,
                  IProductionService& productionService,
                  ISampleRepository& sampleRepo,
                  IMainMenuView& view);
    void run();

private:
    ISubController&     sampleCtrl_;
    ISubController&     orderCtrl_;
    ISubController&     monitoringCtrl_;
    ISubController&     shipmentCtrl_;
    ISubController&     productionLineCtrl_;
    IProductionService& productionService_;
    ISampleRepository&  sampleRepo_;
    IMainMenuView&      view_;
};
