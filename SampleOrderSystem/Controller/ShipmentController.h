#pragma once
#include "../Model/Repository/IOrderRepository.h"
#include "../Model/Repository/ISampleRepository.h"
#include "../View/IShipmentView.h"

class ShipmentController {
public:
    ShipmentController(IOrderRepository& orderRepo,
                       ISampleRepository& sampleRepo,
                       IShipmentView& view);

    void run();
    void listConfirmedOrders();
    void processShipment();

private:
    IOrderRepository&  orderRepo_;
    ISampleRepository& sampleRepo_;
    IShipmentView&     view_;

    static constexpr auto kErrOrderNotFound      = "존재하지 않는 주문 ID입니다.";
    static constexpr auto kErrInsufficientStock  = "재고 부족으로 출고 불가합니다.";
};
