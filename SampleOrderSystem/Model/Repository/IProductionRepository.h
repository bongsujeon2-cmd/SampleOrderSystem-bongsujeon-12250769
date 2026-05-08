#pragma once
#include "../Domain/ProductionJob.h"

class IProductionRepository {
public:
    virtual ~IProductionRepository() = default;
    virtual void enqueue(const ProductionJob& job) = 0;
};
