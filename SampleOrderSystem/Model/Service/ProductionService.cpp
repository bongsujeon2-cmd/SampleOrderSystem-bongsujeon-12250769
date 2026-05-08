#include "ProductionService.h"

ProductionService::ProductionService(
    ISampleRepository&     sampleRepo,
    IOrderRepository&      orderRepo,
    IProductionRepository& productionRepo,
    ITimeProvider&         timeProvider)
    : sampleRepo_(sampleRepo)
    , orderRepo_(orderRepo)
    , productionRepo_(productionRepo)
    , timeProvider_(timeProvider)
{}

void ProductionService::checkAndComplete()
{
    checkAndCompleteInternal(productionRepo_.getState());
}

void ProductionService::checkAndCompleteInternal(ProductionState state)
{
    if (!state.activeJob) return;

    auto job = *state.activeJob;
    int64_t elapsed = static_cast<int64_t>(timeProvider_.now()) - job.startTimeUnix;
    if (elapsed < static_cast<int64_t>(job.totalProductionTimeMin) * 60) return;

    // BR-09: 재고 증가
    auto sampleOpt = sampleRepo_.findById(job.sampleId);
    if (sampleOpt) {
        auto sample = *sampleOpt;
        sample.currentStock += job.actualProductionQty;
        sampleRepo_.update(sample);
    }

    // BR-09: 주문 CONFIRMED 전환
    auto orderOpt = orderRepo_.findById(job.orderId);
    if (orderOpt) {
        auto order = *orderOpt;
        order.status = OrderStatus::CONFIRMED;
        orderRepo_.update(order);
    }

    // activeJob 제거
    state.activeJob = std::nullopt;

    if (!state.queue.empty()) {
        // 다음 job을 꺼내 시작 시간을 기록하여 저장소에 반영 (BR-16)
        auto nextJob = state.queue.front();
        state.queue.pop_front();

        // 저장소에는 현재 시각으로 시작된 상태로 저장
        ProductionState repoState = state;
        repoState.activeJob = nextJob;
        repoState.activeJob->startTimeUnix = static_cast<int64_t>(timeProvider_.now());
        productionRepo_.setState(repoState);

        // BR-17: 재귀 체크 — 원래 startTimeUnix를 유지하여 이미 완료된 job도 처리
        state.activeJob = nextJob;  // original startTimeUnix retained
        checkAndCompleteInternal(state);
    } else {
        productionRepo_.setState(state);
    }
}
