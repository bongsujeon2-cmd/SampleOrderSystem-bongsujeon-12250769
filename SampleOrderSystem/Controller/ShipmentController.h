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
};
