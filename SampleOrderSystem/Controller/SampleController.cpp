#include "SampleController.h"

static constexpr auto kErrDupId   = "이미 존재하는 시료 ID입니다.";
static constexpr auto kErrDupName = "이미 존재하는 시료 이름입니다.";
static constexpr auto kMsgSaved   = "시료가 등록되었습니다.";

SampleController::SampleController(ISampleRepository& repo, ISampleView& view)
    : repo_(repo), view_(view)
{}

void SampleController::registerSample() {
    Sample sample = view_.promptSampleInput();

    if (repo_.existsId(sample.id)) {
        view_.showError(kErrDupId);
        return;
    }

    if (repo_.existsName(sample.name)) {
        view_.showError(kErrDupName);
        return;
    }

    auto error = sample.validate();
    if (!error.empty()) {
        view_.showError(error);
        return;
    }

    repo_.save(sample);
    view_.showSuccess(kMsgSaved);
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
