#pragma once

class IProductionService {
public:
    virtual ~IProductionService() = default;
    virtual void checkAndComplete() = 0;
};
