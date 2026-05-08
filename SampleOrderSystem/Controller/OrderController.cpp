#include "OrderController.h"
#include "../Model/Domain/ProductionJob.h"
#include <cmath>

static constexpr auto kErrUnknownSample  = "등록되지 않은 시료 ID입니다.";
static constexpr auto kErrInvalidQty     = "주문 수량은 1 이상이어야 합니다.";
static constexpr auto kErrUnknownOrder   = "존재하지 않는 주문 ID입니다.";
static constexpr auto kErrUnknownSampleR = "시료 정보를 찾을 수 없습니다.";
static constexpr auto kMsgOrderPlaced    = "주문이 접수되었습니다.";

OrderController::OrderController(IOrderRepository& orderRepo,
                                 ISampleRepository& sampleRepo,
                                 IProductionRepository& productionRepo,
                                 ITimeProvider& timeProvider,
                                 IOrderView& view)
    : orderRepo_(orderRepo)
    , sampleRepo_(sampleRepo)
    , productionRepo_(productionRepo)
    , timeProvider_(timeProvider)
    , view_(view)
{
}

void OrderController::placeOrder()
{
    auto input = view_.promptOrderInput();

    if (input.quantity <= 0) {
        view_.showError(kErrInvalidQty);
        return;
    }

    if (!sampleRepo_.existsId(input.sampleId)) {
        view_.showError(kErrUnknownSample);
        return;
    }

    Order o;
    o.sampleId     = input.sampleId;
    o.customerName = input.customerName;
    o.quantity     = input.quantity;
    o.status       = OrderStatus::RESERVED;
    o.createdAt    = timeProvider_.nowIso8601();

    orderRepo_.create(o);
    view_.showSuccess(kMsgOrderPlaced);
}

void OrderController::approveOrder(const std::string& orderId)
{
    auto orderOpt = orderRepo_.findById(orderId);
    if (!orderOpt) { view_.showError(kErrUnknownOrder); return; }
    auto& order = *orderOpt;

    auto sampleOpt = sampleRepo_.findById(order.sampleId);
    if (!sampleOpt) { view_.showError(kErrUnknownSampleR); return; }
    auto& sample = *sampleOpt;

    if (sample.currentStock >= order.quantity) {
        order.status = OrderStatus::CONFIRMED;
        orderRepo_.update(order);
    } else {
        int shortage  = order.quantity - sample.currentStock;
        int actualQty = static_cast<int>(
            std::ceil(static_cast<double>(shortage) / (sample.yieldRate * 0.9)));
        int totalMin  = sample.avgProductionTime * actualQty;

        ProductionJob job;
        job.orderId                = order.id;
        job.sampleId               = order.sampleId;
        job.shortage               = shortage;
        job.actualProductionQty    = actualQty;
        job.totalProductionTimeMin = totalMin;
        job.startTimeUnix          = 0;

        productionRepo_.enqueue(job);
        order.status = OrderStatus::PRODUCING;
        orderRepo_.update(order);
    }

    view_.showApprovalResult(order.status);
}

void OrderController::rejectOrder(const std::string& orderId)
{
    auto orderOpt = orderRepo_.findById(orderId);
    if (!orderOpt) { view_.showError(kErrUnknownOrder); return; }
    auto& order = *orderOpt;

    order.status = OrderStatus::REJECTED;
    orderRepo_.update(order);
    view_.showApprovalResult(OrderStatus::REJECTED);
}

void OrderController::listReservedOrders()
{
    auto orders  = orderRepo_.findByStatus(OrderStatus::RESERVED);
    auto samples = sampleRepo_.findAll();
    view_.showReservedOrders(orders, samples);
}

void OrderController::processApprovalMenu()
{
    auto reserved = orderRepo_.findByStatus(OrderStatus::RESERVED);
    auto samples  = sampleRepo_.findAll();
    view_.showReservedOrders(reserved, samples);

    if (reserved.empty()) return;

    int sel = view_.promptOrderSelect(static_cast<int>(reserved.size()));
    if (sel <= 0) return;

    int action = view_.promptApproveOrReject();
    if (action == 1) approveOrder(reserved[static_cast<size_t>(sel) - 1].id);
    else if (action == 2) rejectOrder(reserved[static_cast<size_t>(sel) - 1].id);
}

void OrderController::run()
{
    while (true) {
        view_.showSubMenu();
        int ch = view_.getSubMenuChoice();
        switch (ch) {
            case 1: placeOrder(); break;
            case 2: processApprovalMenu(); break;
            case 0: return;
            default: view_.showError("잘못된 선택입니다."); break;
        }
    }
}
