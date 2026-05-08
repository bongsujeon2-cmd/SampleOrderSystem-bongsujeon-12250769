#pragma once
#include "../Model/Repository/ISampleRepository.h"
#include "../View/ISampleView.h"

class SampleController {
public:
    SampleController(ISampleRepository& repo, ISampleView& view);

    void registerSample();
    void listSamples();
    void searchSamples();
    void run();

private:
    ISampleRepository& repo_;
    ISampleView&       view_;
};
