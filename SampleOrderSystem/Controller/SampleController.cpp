#include "SampleController.h"

SampleController::SampleController(ISampleRepository& repo, ISampleView& view)
    : repo_(repo), view_(view)
{}

void SampleController::registerSample() {
    Sample sample = view_.promptSampleInput();

    if (repo_.existsId(sample.id)) {
        view_.showError("이미 존재하는 ID입니다: " + sample.id);
        return;
    }

    if (repo_.existsName(sample.name)) {
        view_.showError("이미 존재하는 이름입니다: " + sample.name);
        return;
    }

    if (sample.yieldRate <= 0.0 || sample.yieldRate > 1.0) {
        view_.showError("수율은 0 초과 1 이하 값이어야 합니다.");
        return;
    }

    repo_.save(sample);
    view_.showSuccess("시료가 등록되었습니다: " + sample.name);
}

void SampleController::listSamples() {
    auto samples = repo_.findAll();
    view_.showSampleList(samples);
}

void SampleController::searchSamples() {
    std::string keyword = view_.promptKeyword();
    auto results = repo_.searchByName(keyword);
    view_.showSearchResult(results);
}

void SampleController::run() {
    while (true) {
        view_.showSubMenu();
        int choice = view_.getSubMenuChoice();
        switch (choice) {
            case 1: registerSample(); break;
            case 2: listSamples();    break;
            case 3: searchSamples();  break;
            case 0: return;
            default: view_.showError("잘못된 선택입니다."); break;
        }
    }
}
