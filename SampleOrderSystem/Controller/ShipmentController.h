#pragma once
#include "ISubController.h"
#include "../Model/Repository/IOrderRepository.h"
#include "../Model/Repository/ISampleRepository.h"
#include "../View/IShipmentView.h"

class ShipmentController : public ISubController {
public:
    ShipmentController(IOrderRepository& orderRepo,
                       ISampleRepository& sampleRepo,
                       IShipmentView& view);

    void run() override;
    void listConfirmedOrders();
    void processShipment();

private:
    IOrderRepository&  orderRepo_;
    ISampleRepository& sampleRepo_;
    IShipmentView&     view_;
};
