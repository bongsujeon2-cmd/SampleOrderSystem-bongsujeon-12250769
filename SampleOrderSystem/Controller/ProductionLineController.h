#pragma once
#include "../Model/Service/IProductionService.h"
#include "../Model/Repository/IProductionRepository.h"
#include "../Model/Repository/ISampleRepository.h"
#include "../Model/Service/ITimeProvider.h"
#include "../View/IProductionLineView.h"

class ProductionLineController {
public:
    ProductionLineController(IProductionService&    productionService,
                             IProductionRepository& productionRepo,
                             ISampleRepository&     sampleRepo,
                             ITimeProvider&         timeProvider,
                             IProductionLineView&   view,
                             bool                   isMockMode = false);

    ProductionLineController(const ProductionLineController&) = delete;
    ProductionLineController& operator=(const ProductionLineController&) = delete;

    void run();
    void showStatus();
    void showQueue();
    void advanceTime(int minutes);

private:
    IProductionService&    productionService_;
    IProductionRepository& productionRepo_;
    ISampleRepository&     sampleRepo_;
    ITimeProvider&         timeProvider_;
    IProductionLineView&   view_;
    bool                   isMockMode_;
};
