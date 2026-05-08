#pragma once
#include "../Domain/ProductionJob.h"

class IProductionRepository {
public:
    virtual ~IProductionRepository() = default;
    virtual ProductionState getState() const = 0;
    virtual void            setState(const ProductionState& state) = 0;
    virtual void            enqueue(const ProductionJob& job) = 0;
    virtual void            clearAll() = 0;
};
