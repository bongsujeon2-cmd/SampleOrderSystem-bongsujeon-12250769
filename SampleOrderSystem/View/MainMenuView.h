#pragma once
#include "IMainMenuView.h"

class MainMenuView : public IMainMenuView {
public:
    void showMainMenu(const std::vector<Sample>& samples) override;
    int  getMenuChoice() override;
    void showInvalidInput() override;
};
