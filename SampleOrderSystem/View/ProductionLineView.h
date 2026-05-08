#pragma once
#include "IProductionLineView.h"
#include <string>
#include <vector>

class ProductionLineView : public IProductionLineView {
public:
    void showSubMenu(bool isMockMode) override;
    int  getSubMenuChoice() override;
    void showActiveJob(const ProductionJob& job,
                       const std::string& sampleName,
                       int elapsedMinutes,
                       std::string expectedFinish) override;
    void showNoActiveJob() override;
    void showQueue(const std::vector<ProductionJob>& queue,
                   const std::vector<std::string>& sampleNames) override;
    int  promptAdvanceMinutes() override;
    void showError(const std::string& msg) override;
};
