#include "ShipmentController.h"

static constexpr auto kErrOrderNotFound     = "존재하지 않는 주문 ID입니다.";
static constexpr auto kErrSampleNotFound    = "시료 정보를 찾을 수 없습니다.";
static constexpr auto kErrNotConfirmed      = "CONFIRMED 상태의 주문만 출고 처리할 수 있습니다.";
static constexpr auto kErrInsufficientStock = "재고 부족으로 출고 불가합니다.";

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

    if (order.status != OrderStatus::CONFIRMED) {
        view_.showError(kErrNotConfirmed);
        return;
    }

    auto sampleOpt = sampleRepo_.findById(order.sampleId);
    if (!sampleOpt) {
        view_.showError(kErrSampleNotFound);
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
