#include "ProductionLineController.h"
#include "../Model/Service/MockTimeProvider.h"
#include "../Model/Service/TimeUtils.h"
#include <vector>
#include <string>

ProductionLineController::ProductionLineController(
    IProductionService&    productionService,
    IProductionRepository& productionRepo,
    ISampleRepository&     sampleRepo,
    ITimeProvider&         timeProvider,
    IProductionLineView&   view,
    bool                   isMockMode)
    : productionService_(productionService)
    , productionRepo_(productionRepo)
    , sampleRepo_(sampleRepo)
    , timeProvider_(timeProvider)
    , view_(view)
    , isMockMode_(isMockMode)
{}

void ProductionLineController::showStatus()
{
    productionService_.checkAndComplete();

    auto state = productionRepo_.getState();
    if (!state.activeJob) {
        view_.showNoActiveJob();
        return;
    }

    const auto& job = *state.activeJob;

    std::string sampleName;
    if (auto s = sampleRepo_.findById(job.sampleId)) {
        sampleName = s->name;
    }

    int elapsed = static_cast<int>(
        (timeProvider_.now() - static_cast<time_t>(job.startTimeUnix)) / 60);

    time_t finishTime = static_cast<time_t>(
        job.startTimeUnix + static_cast<int64_t>(job.totalProductionTimeMin) * 60LL);

    view_.showActiveJob(job, sampleName, elapsed, TimeUtils::toIso8601(finishTime));
}

void ProductionLineController::showQueue()
{
    auto state = productionRepo_.getState();

    std::vector<std::string> names;
    names.reserve(state.queue.size());
    for (const auto& job : state.queue) {
        std::string name;
        if (auto s = sampleRepo_.findById(job.sampleId)) {
            name = s->name;
        }
        names.push_back(name);
    }

    std::vector<ProductionJob> queueVec(state.queue.begin(), state.queue.end());
    view_.showQueue(queueVec, names);
}

void ProductionLineController::advanceTime(int minutes)
{
    if (!isMockMode_) return;
    if (auto* mock = dynamic_cast<MockTimeProvider*>(&timeProvider_))
        mock->advance(minutes);
    productionService_.checkAndComplete();
}

void ProductionLineController::run()
{
    while (true) {
        view_.showSubMenu(isMockMode_);
        int ch = view_.getSubMenuChoice();
        switch (ch) {
            case 1:
                showStatus();
                break;
            case 2:
                showQueue();
                break;
            case 3:
                if (isMockMode_) {
                    int min = view_.promptAdvanceMinutes();
                    advanceTime(min);
                } else {
                    view_.showError("잘못된 선택입니다.");
                }
                break;
            case 0:
                return;
            default:
                view_.showError("잘못된 선택입니다.");
                break;
        }
    }
}
