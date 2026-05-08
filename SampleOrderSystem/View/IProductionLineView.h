#pragma once
#include "../Model/Domain/ProductionJob.h"
#include <string>
#include <vector>

class IProductionLineView {
public:
    virtual ~IProductionLineView() = default;
    virtual void showSubMenu(bool isMockMode) = 0;
    virtual int  getSubMenuChoice() = 0;
    virtual void showActiveJob(const ProductionJob& job,
                               const std::string& sampleName,
                               int elapsedMinutes,
                               const std::string& expectedFinish) = 0;
    virtual void showNoActiveJob() = 0;
    virtual void showQueue(const std::vector<ProductionJob>& queue,
                           const std::vector<std::string>& sampleNames) = 0;
    virtual int  promptAdvanceMinutes() = 0;
    virtual void showError(const std::string& msg) = 0;
};
