#pragma once
#include "ISubController.h"
#include "../Model/Repository/IOrderRepository.h"
#include "../Model/Repository/ISampleRepository.h"
#include "../Model/Repository/IProductionRepository.h"
#include "../Model/Service/ITimeProvider.h"
#include "../View/IOrderView.h"

class OrderController : public ISubController {
public:
    OrderController(IOrderRepository& orderRepo,
                    ISampleRepository& sampleRepo,
                    IProductionRepository& productionRepo,
                    ITimeProvider& timeProvider,
                    IOrderView& view);

    void run() override;
    void placeOrder();
    void processApprovalMenu();
    void approveOrder(const std::string& orderId);
    void rejectOrder(const std::string& orderId);
    void listReservedOrders();

private:
    IOrderRepository&      orderRepo_;
    ISampleRepository&     sampleRepo_;
    IProductionRepository& productionRepo_;
    ITimeProvider&         timeProvider_;
    IOrderView&            view_;
};
