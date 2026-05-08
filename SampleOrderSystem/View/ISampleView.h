#pragma once
#include "../Model/Domain/Sample.h"
#include <vector>
#include <string>

class ISampleView {
public:
    virtual ~ISampleView() = default;
    virtual void        showSubMenu() = 0;
    virtual int         getSubMenuChoice() = 0;
    virtual Sample      promptSampleInput() = 0;
    virtual void        showSampleList(const std::vector<Sample>& samples) = 0;
    virtual void        showSearchResult(const std::vector<Sample>& results) = 0;
    virtual void        showError(const std::string& msg) = 0;
    virtual void        showSuccess(const std::string& msg) = 0;
    virtual std::string promptKeyword() = 0;
};
