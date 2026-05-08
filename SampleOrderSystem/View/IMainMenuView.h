#pragma once
#include "../Model/Domain/Sample.h"
#include <vector>

class IMainMenuView {
public:
    virtual ~IMainMenuView() = default;
    virtual void showMainMenu(const std::vector<Sample>& samples) = 0;
    virtual int  getMenuChoice() = 0;
    virtual void showInvalidInput() = 0;
};
