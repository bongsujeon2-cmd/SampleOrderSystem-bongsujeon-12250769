#pragma once
#include "../Model/Repository/IOrderRepository.h"
#include "../Model/Repository/ISampleRepository.h"
#include "../Model/Repository/IProductionRepository.h"
#include "../Model/Service/ITimeProvider.h"
#include "../View/IOrderView.h"

class OrderController {
public:
    OrderController(IOrderRepository& orderRepo,
                    ISampleRepository& sampleRepo,
                    IProductionRepository& productionRepo,
                    ITimeProvider& timeProvider,
                    IOrderView& view);

    void run();
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

    static constexpr auto kErrUnknownSample = "등록되지 않은 시료 ID입니다.";
    static constexpr auto kErrInvalidQty    = "주문 수량은 1 이상이어야 합니다.";
    static constexpr auto kMsgOrderPlaced   = "주문이 접수되었습니다.";
};
