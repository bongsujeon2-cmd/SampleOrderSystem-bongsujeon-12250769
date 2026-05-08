#pragma once
#include "ISubController.h"
#include "../Model/Repository/ISampleRepository.h"
#include "../View/ISampleView.h"

class SampleController : public ISubController {
public:
    SampleController(ISampleRepository& repo, ISampleView& view);

    void registerSample();
    void listSamples();
    void searchSamples();
    void run() override;

private:
    ISampleRepository& repo_;
    ISampleView&       view_;
};
