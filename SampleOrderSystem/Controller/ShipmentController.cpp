#include "ShipmentController.h"

ShipmentController::ShipmentController(IOrderRepository& orderRepo,
                                        ISampleRepository& sampleRepo,
                                        IShipmentView& view)
    : orderRepo_(orderRepo)
    , sampleRepo_(sampleRepo)
    , view_(view)
{}

void ShipmentController::processShipment() {
    auto orderId = view_.promptOrderIdInput();

    auto orderOpt = orderRepo_.findById(orderId);
    if (!orderOpt) {
        view_.showError(kErrOrderNotFound);
        return;
    }
    auto order = *orderOpt;

    auto sampleOpt = sampleRepo_.findById(order.sampleId);
    if (!sampleOpt) {
        view_.showError(kErrOrderNotFound);
        return;
    }
    auto sample = *sampleOpt;

    if (sample.currentStock < order.quantity) {
        view_.showError(kErrInsufficientStock);
        return;
    }

    sample.currentStock -= order.quantity;
    sampleRepo_.update(sample);        // BR-10: 출고 시점에만 차감

    order.status = OrderStatus::RELEASE;
    orderRepo_.update(order);          // BR-11

    view_.showShipmentSuccess(orderId, order.quantity);
}

void ShipmentController::listConfirmedOrders() {
    auto orders = orderRepo_.findByStatus(OrderStatus::CONFIRMED);
    if (orders.empty()) {
        view_.showNoConfirmedOrders();
        return;
    }

    auto samples = sampleRepo_.findAll();
    view_.showConfirmedOrders(orders, samples);
}

void ShipmentController::run() {
    while (true) {
        view_.showSubMenu();
        int ch = view_.getSubMenuChoice();
        switch (ch) {
        case 1:
            listConfirmedOrders();
            break;
        case 2:
            processShipment();
            break;
        case 0:
            return;
        default:
            view_.showError("잘못된 선택입니다.");
            break;
        }
    }
}
