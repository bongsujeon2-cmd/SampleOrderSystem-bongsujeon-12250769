#pragma once
#include "../Repository/ISampleRepository.h"
#include "../Repository/IOrderRepository.h"
#include "../Repository/IProductionRepository.h"
#include "ITimeProvider.h"

class ProductionService {
public:
    ProductionService(ISampleRepository&, IOrderRepository&,
                      IProductionRepository&, ITimeProvider&);
    void checkAndComplete();
private:
    ISampleRepository&     sampleRepo_;
    IOrderRepository&      orderRepo_;
    IProductionRepository& productionRepo_;
    ITimeProvider&         timeProvider_;

    bool isJobComplete(const ProductionJob& job) const;
    void completeCurrentJob(ProductionState& state);
    void startNextJob(ProductionState& state);
};
