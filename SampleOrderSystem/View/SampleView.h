#pragma once
#include "ISampleView.h"
#include <vector>
#include <string>

class SampleView : public ISampleView {
public:
    void        showSubMenu() override;
    int         getSubMenuChoice() override;
    Sample      promptSampleInput() override;
    void        showSampleList(const std::vector<Sample>& samples) override;
    void        showSearchResult(const std::vector<Sample>& results) override;
    void        showError(const std::string& msg) override;
    void        showSuccess(const std::string& msg) override;
    std::string promptKeyword() override;
};
