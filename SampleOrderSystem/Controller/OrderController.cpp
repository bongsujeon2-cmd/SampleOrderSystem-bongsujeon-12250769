#include "OrderController.h"
#include "../Model/Domain/ProductionJob.h"
#include <cmath>

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
    auto [sampleId, customerName, quantity] = view_.promptOrderInput();

    if (quantity <= 0) {
        view_.showError(kErrInvalidQty);
        return;
    }

    if (!sampleRepo_.existsId(sampleId)) {
        view_.showError(kErrUnknownSample);
        return;
    }

    Order o;
    o.sampleId     = sampleId;
    o.customerName = customerName;
    o.quantity     = quantity;
    o.status       = OrderStatus::RESERVED;
    o.createdAt    = timeProvider_.nowIso8601();

    orderRepo_.create(o);
    view_.showSuccess(kMsgOrderPlaced);
}

void OrderController::approveOrder(const std::string& orderId)
{
    auto order  = orderRepo_.findById(orderId).value();
    auto sample = sampleRepo_.findById(order.sampleId).value();

    if (sample.currentStock >= order.quantity) {
        order.status = OrderStatus::CONFIRMED;
        orderRepo_.update(order);
    } else {
        int shortage  = order.quantity - sample.currentStock;
        int actualQty = static_cast<int>(
            std::ceil(shortage / (sample.yieldRate * sample.yieldRate)));
        int totalMin  = sample.avgProductionTime * actualQty;

        ProductionJob job;
        job.orderId               = order.id;
        job.sampleId              = order.sampleId;
        job.shortage              = shortage;
        job.actualProductionQty   = actualQty;
        job.totalProductionTimeMin = totalMin;
        job.startTimeUnix         = 0;

        productionRepo_.enqueue(job);
        order.status = OrderStatus::PRODUCING;
        orderRepo_.update(order);
    }

    view_.showApprovalResult(order.status);
}

void OrderController::rejectOrder(const std::string& orderId)
{
    auto order   = orderRepo_.findById(orderId).value();
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
    listReservedOrders();

    auto reserved = orderRepo_.findByStatus(OrderStatus::RESERVED);
    if (reserved.empty()) {
        return;
    }

    int sel = view_.promptOrderSelect(static_cast<int>(reserved.size()));
    if (sel <= 0) {
        return;
    }

    approveOrder(reserved[static_cast<size_t>(sel) - 1].id);
}

void OrderController::run()
{
    while (true) {
        view_.showSubMenu();
        int ch = view_.getSubMenuChoice();
        if (ch == 0) break;
        if (ch == 1) placeOrder();
        if (ch == 2) processApprovalMenu();
    }
}
