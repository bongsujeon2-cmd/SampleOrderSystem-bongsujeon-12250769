#include "ProductionService.h"
#include <stdexcept>

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

bool ProductionService::isJobComplete(const ProductionJob& job) const
{
    // elapsed는 int64_t로 캐스팅 (현재 플랫폼에서 time_t는 int64_t와 동일)
    int64_t elapsed = static_cast<int64_t>(timeProvider_.now()) - job.startTimeUnix;
    return elapsed >= static_cast<int64_t>(job.totalProductionTimeMin) * 60;
}

void ProductionService::completeCurrentJob(ProductionState& state)
{
    const ProductionJob& job = *state.activeJob;

    // BR-09: 재고 증가
    auto sampleOpt = sampleRepo_.findById(job.sampleId);
    if (!sampleOpt)
        throw std::runtime_error("생산 완료 처리: 시료를 찾을 수 없음 - " + job.sampleId);
    auto sample = *sampleOpt;
    sample.currentStock += job.actualProductionQty;
    sampleRepo_.update(sample);

    // BR-09: 주문 CONFIRMED 전환
    auto orderOpt = orderRepo_.findById(job.orderId);
    if (!orderOpt)
        throw std::runtime_error("생산 완료 처리: 주문을 찾을 수 없음 - " + job.orderId);
    auto order = *orderOpt;
    order.status = OrderStatus::CONFIRMED;
    orderRepo_.update(order);

    // activeJob 제거
    state.activeJob = std::nullopt;
}

void ProductionService::startNextJob(ProductionState& state)
{
    auto nextJob = state.queue.front();
    state.queue.pop_front();
    nextJob.startTimeUnix = static_cast<int64_t>(timeProvider_.now());
    state.activeJob = nextJob;
}

void ProductionService::checkAndComplete()
{
    while (true) {
        auto state = productionRepo_.getState();
        if (!state.activeJob) return;
        if (!isJobComplete(*state.activeJob)) return;

        completeCurrentJob(state);          // 재고+주문 업데이트, activeJob=nullopt

        if (!state.queue.empty()) {
            startNextJob(state);            // queue.front() → activeJob, startTimeUnix=now()
            productionRepo_.setState(state);
            // 루프 계속 → 다음 iteration에서 새 activeJob 완료 여부 체크
        } else {
            productionRepo_.setState(state);
            return;
        }
    }
}
