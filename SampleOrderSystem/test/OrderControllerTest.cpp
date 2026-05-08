// OrderControllerTest.cpp
// OrderController 단위 테스트 (TDD — 구현 없이 먼저 작성)
// 이 시점에서 구현이 없으므로 테스트는 FAIL 상태가 정상

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Model/Repository/ISampleRepository.h"
#include "../Model/Repository/IOrderRepository.h"
#include "../Model/Repository/IProductionRepository.h"
#include "../Model/Service/ITimeProvider.h"
#include "../View/IOrderView.h"
#include "../Controller/OrderController.h"
#include <cmath>

using ::testing::Return;
using ::testing::ReturnArg;
using ::testing::_;
using ::testing::Eq;
using ::testing::SaveArg;

// -----------------------------------------------------------------------
// Mock 클래스
// -----------------------------------------------------------------------

class MockSampleRepository : public ISampleRepository {
public:
    MOCK_METHOD(bool, save, (const Sample&), (override));
    MOCK_METHOD(bool, update, (const Sample&), (override));
    MOCK_METHOD(std::optional<Sample>, findById, (const std::string&), (const, override));
    MOCK_METHOD(std::vector<Sample>, findAll, (), (const, override));
    MOCK_METHOD(std::vector<Sample>, searchByName, (const std::string&), (const, override));
    MOCK_METHOD(bool, existsId, (const std::string&), (const, override));
    MOCK_METHOD(bool, existsName, (const std::string&), (const, override));
};

class MockOrderRepository : public IOrderRepository {
public:
    MOCK_METHOD(std::string, create, (const Order&), (override));
    MOCK_METHOD(std::optional<Order>, findById, (const std::string&), (const, override));
    MOCK_METHOD(std::vector<Order>, findAll, (), (const, override));
    MOCK_METHOD(std::vector<Order>, findByStatus, (OrderStatus), (const, override));
    MOCK_METHOD(bool, update, (const Order&), (override));
};

class MockProductionRepository : public IProductionRepository {
public:
    MOCK_METHOD(void, enqueue, (const ProductionJob&), (override));
};

class MockTimeProvider : public ITimeProvider {
public:
    MOCK_METHOD(time_t, now, (), (const, override));
    MOCK_METHOD(std::string, nowIso8601, (), (const, override));
};

class MockOrderView : public IOrderView {
public:
    MOCK_METHOD(void, showSubMenu, (), (override));
    MOCK_METHOD(int, getSubMenuChoice, (), (override));
    MOCK_METHOD(OrderInput, promptOrderInput, (), (override));
    MOCK_METHOD(void, showReservedOrders, (const std::vector<Order>&, const std::vector<Sample>&), (override));
    MOCK_METHOD(int, promptOrderSelect, (int), (override));
    MOCK_METHOD(int, promptApproveOrReject, (), (override));
    MOCK_METHOD(void, showApprovalResult, (OrderStatus), (override));
    MOCK_METHOD(void, showError, (const std::string&), (override));
    MOCK_METHOD(void, showSuccess, (const std::string&), (override));
};

// -----------------------------------------------------------------------
// 헬퍼
// -----------------------------------------------------------------------

/// 테스트용 Order 객체 생성 헬퍼
static Order makeOrder(
    const std::string& id,
    const std::string& sampleId,
    int qty,
    OrderStatus status = OrderStatus::RESERVED)
{
    Order o;
    o.id           = id;
    o.sampleId     = sampleId;
    o.customerName = "TestCustomer";
    o.quantity     = qty;
    o.status       = status;
    o.createdAt    = "2026-05-08T00:00:00";
    return o;
}

/// 테스트용 Sample 객체 생성 헬퍼
static Sample makeSample(
    const std::string& id,
    int stock,
    double yieldRate = 0.9,
    int avgTime = 10)
{
    Sample s;
    s.id                = id;
    s.name              = "TestSample";
    s.avgProductionTime = avgTime;
    s.yieldRate         = yieldRate;
    s.currentStock      = stock;
    return s;
}

// -----------------------------------------------------------------------
// Fixture
// -----------------------------------------------------------------------

class OrderControllerTest : public ::testing::Test {
protected:
    MockSampleRepository     mockSampleRepo;
    MockOrderRepository      mockOrderRepo;
    MockProductionRepository mockProductionRepo;
    MockTimeProvider         mockTime;
    MockOrderView            mockView;

    OrderController makeController() {
        return OrderController(mockOrderRepo, mockSampleRepo, mockProductionRepo, mockTime, mockView);
    }
};

// -----------------------------------------------------------------------
// TC-01: placeOrder() — 유효한 sampleId로 RESERVED 주문 생성
// existsId("S-001")=true → create() 1회 호출,
// 생성된 Order의 status=RESERVED, sampleId/customerName/quantity 검증
// -----------------------------------------------------------------------
TEST_F(OrderControllerTest, PlaceOrder_ValidSampleId_CreatesReservedOrder)
{
    const std::string sampleId     = "S-001";
    const std::string customerName = "홍길동";
    const int         quantity     = 10;
    const std::string fixedTime    = "2026-05-08T00:00:00";

    EXPECT_CALL(mockView, promptOrderInput())
        .Times(1)
        .WillOnce(Return(OrderInput{sampleId, customerName, quantity}));

    EXPECT_CALL(mockSampleRepo, existsId(Eq(sampleId)))
        .Times(1)
        .WillOnce(Return(true));

    EXPECT_CALL(mockTime, nowIso8601())
        .Times(1)
        .WillOnce(Return(fixedTime));

    Order capturedOrder;
    EXPECT_CALL(mockOrderRepo, create(_))
        .Times(1)
        .WillOnce([&capturedOrder](const Order& o) {
            capturedOrder = o;
            return "ORD-001";
        });

    EXPECT_CALL(mockView, showSuccess(_))
        .Times(1);

    EXPECT_CALL(mockView, showError(_))
        .Times(0);

    auto controller = makeController();
    controller.placeOrder();

    EXPECT_EQ(capturedOrder.sampleId,     sampleId);
    EXPECT_EQ(capturedOrder.customerName, customerName);
    EXPECT_EQ(capturedOrder.quantity,     quantity);
    EXPECT_EQ(capturedOrder.status,       OrderStatus::RESERVED);
    EXPECT_EQ(capturedOrder.createdAt,    fixedTime);
}

// -----------------------------------------------------------------------
// TC-02: placeOrder() — 미등록 sampleId → showError() 1회, create() 미호출
// -----------------------------------------------------------------------
TEST_F(OrderControllerTest, PlaceOrder_UnregisteredSampleId_ShowsError)
{
    const std::string sampleId     = "UNKNOWN";
    const std::string customerName = "홍길동";
    const int         quantity     = 10;

    EXPECT_CALL(mockView, promptOrderInput())
        .Times(1)
        .WillOnce(Return(OrderInput{sampleId, customerName, quantity}));

    EXPECT_CALL(mockSampleRepo, existsId(Eq(sampleId)))
        .Times(1)
        .WillOnce(Return(false));

    EXPECT_CALL(mockOrderRepo, create(_))
        .Times(0);

    EXPECT_CALL(mockView, showError(_))
        .Times(1);

    EXPECT_CALL(mockView, showSuccess(_))
        .Times(0);

    auto controller = makeController();
    controller.placeOrder();
}

// -----------------------------------------------------------------------
// TC-03: placeOrder() — quantity=0 → showError() 1회, create() 미호출
// -----------------------------------------------------------------------
TEST_F(OrderControllerTest, PlaceOrder_ZeroQuantity_ShowsError)
{
    const std::string sampleId     = "S-001";
    const std::string customerName = "홍길동";
    const int         quantity     = 0;

    EXPECT_CALL(mockView, promptOrderInput())
        .Times(1)
        .WillOnce(Return(OrderInput{sampleId, customerName, quantity}));

    EXPECT_CALL(mockOrderRepo, create(_))
        .Times(0);

    EXPECT_CALL(mockView, showError(_))
        .Times(1);

    EXPECT_CALL(mockView, showSuccess(_))
        .Times(0);

    auto controller = makeController();
    controller.placeOrder();
}

// -----------------------------------------------------------------------
// TC-04: placeOrder() — quantity=-1 → showError() 1회, create() 미호출
// -----------------------------------------------------------------------
TEST_F(OrderControllerTest, PlaceOrder_NegativeQuantity_ShowsError)
{
    const std::string sampleId     = "S-001";
    const std::string customerName = "홍길동";
    const int         quantity     = -1;

    EXPECT_CALL(mockView, promptOrderInput())
        .Times(1)
        .WillOnce(Return(OrderInput{sampleId, customerName, quantity}));

    EXPECT_CALL(mockOrderRepo, create(_))
        .Times(0);

    EXPECT_CALL(mockView, showError(_))
        .Times(1);

    EXPECT_CALL(mockView, showSuccess(_))
        .Times(0);

    auto controller = makeController();
    controller.placeOrder();
}

// -----------------------------------------------------------------------
// TC-05: approveOrder() — 재고 충분 (stock=50, quantity=20)
// → order.status=CONFIRMED, update() 호출, enqueue() 미호출,
//   ISampleRepository::update() 미호출
// -----------------------------------------------------------------------
TEST_F(OrderControllerTest, ApproveOrder_StockSufficient_BecomesConfirmed)
{
    const std::string orderId  = "ORD-001";
    const std::string sampleId = "S-001";
    const int         quantity = 20;
    const int         stock    = 50;

    Order  order  = makeOrder(orderId, sampleId, quantity, OrderStatus::RESERVED);
    Sample sample = makeSample(sampleId, stock, 0.9, 10);

    EXPECT_CALL(mockOrderRepo, findById(Eq(orderId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(order)));

    EXPECT_CALL(mockSampleRepo, findById(Eq(sampleId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(sample)));

    Order updatedOrder;
    EXPECT_CALL(mockOrderRepo, update(_))
        .Times(1)
        .WillOnce([&updatedOrder](const Order& o) {
            updatedOrder = o;
            return true;
        });

    // 재고 충분 → enqueue() 호출 없음
    EXPECT_CALL(mockProductionRepo, enqueue(_))
        .Times(0);

    // 재고 차감은 출고 시 → ISampleRepository::update() 호출 없음
    EXPECT_CALL(mockSampleRepo, update(_))
        .Times(0);

    EXPECT_CALL(mockView, showApprovalResult(Eq(OrderStatus::CONFIRMED)))
        .Times(1);

    auto controller = makeController();
    controller.approveOrder(orderId);

    EXPECT_EQ(updatedOrder.status, OrderStatus::CONFIRMED);
}

// -----------------------------------------------------------------------
// TC-06: approveOrder() — 재고 부족 (stock=5, quantity=20, yieldRate=0.9, avgTime=10)
// → order.status=PRODUCING, enqueue() 1회 호출
// -----------------------------------------------------------------------
TEST_F(OrderControllerTest, ApproveOrder_StockInsufficient_BecomesProducing)
{
    const std::string orderId  = "ORD-002";
    const std::string sampleId = "S-001";
    const int         quantity = 20;
    const int         stock    = 5;
    const double      yield    = 0.9;
    const int         avgTime  = 10;

    Order  order  = makeOrder(orderId, sampleId, quantity, OrderStatus::RESERVED);
    Sample sample = makeSample(sampleId, stock, yield, avgTime);

    EXPECT_CALL(mockOrderRepo, findById(Eq(orderId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(order)));

    EXPECT_CALL(mockSampleRepo, findById(Eq(sampleId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(sample)));

    Order updatedOrder;
    EXPECT_CALL(mockOrderRepo, update(_))
        .Times(1)
        .WillOnce([&updatedOrder](const Order& o) {
            updatedOrder = o;
            return true;
        });

    EXPECT_CALL(mockProductionRepo, enqueue(_))
        .Times(1);

    EXPECT_CALL(mockView, showApprovalResult(Eq(OrderStatus::PRODUCING)))
        .Times(1);

    auto controller = makeController();
    controller.approveOrder(orderId);

    EXPECT_EQ(updatedOrder.status, OrderStatus::PRODUCING);
}

// -----------------------------------------------------------------------
// TC-07: approveOrder() — 생산량 계산 검증
// shortage=15, yieldRate=0.9
// actualQty = ceil(15 / (0.9 * 0.9)) = ceil(15 / 0.81) = ceil(18.518...) = 19
// totalMinutes = avgTime * actualQty = avgTime * 19
// -----------------------------------------------------------------------
TEST_F(OrderControllerTest, ApproveOrder_StockInsufficient_ProductionQtyCalculation)
{
    const std::string orderId   = "ORD-003";
    const std::string sampleId  = "S-002";
    const int         stock     = 5;
    const int         quantity  = 20;   // shortage = 20 - 5 = 15
    const double      yield     = 0.9;
    const int         avgTime   = 10;
    const int         expectedActualQty = 19; // ceil(15 / 0.81)
    const int         expectedTotalMin  = avgTime * expectedActualQty;

    Order  order  = makeOrder(orderId, sampleId, quantity, OrderStatus::RESERVED);
    Sample sample = makeSample(sampleId, stock, yield, avgTime);

    EXPECT_CALL(mockOrderRepo, findById(Eq(orderId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(order)));

    EXPECT_CALL(mockSampleRepo, findById(Eq(sampleId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(sample)));

    EXPECT_CALL(mockOrderRepo, update(_))
        .Times(1)
        .WillOnce(Return(true));

    ProductionJob capturedJob;
    EXPECT_CALL(mockProductionRepo, enqueue(_))
        .Times(1)
        .WillOnce([&capturedJob](const ProductionJob& job) {
            capturedJob = job;
        });

    EXPECT_CALL(mockView, showApprovalResult(_))
        .Times(1);

    auto controller = makeController();
    controller.approveOrder(orderId);

    EXPECT_EQ(capturedJob.actualProductionQty,    expectedActualQty);
    EXPECT_EQ(capturedJob.totalProductionTimeMin, expectedTotalMin);
}

// -----------------------------------------------------------------------
// TC-08: approveOrder() — enqueue() 호출 시 ProductionJob 전체 필드 검증
// shortage=15 (stock=5, qty=20), yieldRate=0.9, avgTime=10
// orderId, sampleId, shortage, actualProductionQty, totalProductionTimeMin
// -----------------------------------------------------------------------
TEST_F(OrderControllerTest, ApproveOrder_StockInsufficient_EnqueuedJobFields)
{
    const std::string orderId   = "ORD-004";
    const std::string sampleId  = "S-003";
    const int         stock     = 5;
    const int         quantity  = 20;
    const double      yield     = 0.9;
    const int         avgTime   = 10;
    const int         shortage  = quantity - stock;               // 15
    const int         actualQty = 19;                             // ceil(15 / 0.81)
    const int         totalMin  = avgTime * actualQty;            // 190

    Order  order  = makeOrder(orderId, sampleId, quantity, OrderStatus::RESERVED);
    Sample sample = makeSample(sampleId, stock, yield, avgTime);

    EXPECT_CALL(mockOrderRepo, findById(Eq(orderId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(order)));

    EXPECT_CALL(mockSampleRepo, findById(Eq(sampleId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(sample)));

    EXPECT_CALL(mockOrderRepo, update(_))
        .Times(1)
        .WillOnce(Return(true));

    ProductionJob capturedJob;
    EXPECT_CALL(mockProductionRepo, enqueue(_))
        .Times(1)
        .WillOnce([&capturedJob](const ProductionJob& job) {
            capturedJob = job;
        });

    EXPECT_CALL(mockView, showApprovalResult(_))
        .Times(1);

    auto controller = makeController();
    controller.approveOrder(orderId);

    EXPECT_EQ(capturedJob.orderId,               orderId);
    EXPECT_EQ(capturedJob.sampleId,              sampleId);
    EXPECT_EQ(capturedJob.shortage,              shortage);
    EXPECT_EQ(capturedJob.actualProductionQty,   actualQty);
    EXPECT_EQ(capturedJob.totalProductionTimeMin, totalMin);
}

// -----------------------------------------------------------------------
// TC-09: rejectOrder() — order.status=REJECTED, IOrderRepository::update() 호출
// -----------------------------------------------------------------------
TEST_F(OrderControllerTest, RejectOrder_BecomesRejected)
{
    const std::string orderId  = "ORD-005";
    const std::string sampleId = "S-001";
    const int         quantity = 10;

    Order order = makeOrder(orderId, sampleId, quantity, OrderStatus::RESERVED);

    EXPECT_CALL(mockOrderRepo, findById(Eq(orderId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(order)));

    Order updatedOrder;
    EXPECT_CALL(mockOrderRepo, update(_))
        .Times(1)
        .WillOnce([&updatedOrder](const Order& o) {
            updatedOrder = o;
            return true;
        });

    EXPECT_CALL(mockProductionRepo, enqueue(_))
        .Times(0);

    EXPECT_CALL(mockView, showApprovalResult(Eq(OrderStatus::REJECTED)))
        .Times(1);

    auto controller = makeController();
    controller.rejectOrder(orderId);

    EXPECT_EQ(updatedOrder.status, OrderStatus::REJECTED);
}

// -----------------------------------------------------------------------
// TC-10: listReservedOrders() — findByStatus(RESERVED) 1회 호출,
//         showReservedOrders() 1회 호출
// -----------------------------------------------------------------------
TEST_F(OrderControllerTest, ListReservedOrders_CallsFindByStatusReserved)
{
    std::vector<Order> reservedOrders = {
        makeOrder("ORD-001", "S-001", 10, OrderStatus::RESERVED),
        makeOrder("ORD-002", "S-002", 20, OrderStatus::RESERVED),
    };

    std::vector<Sample> samples = {
        makeSample("S-001", 100, 0.9, 10),
        makeSample("S-002", 50,  0.85, 5),
    };

    EXPECT_CALL(mockOrderRepo, findByStatus(Eq(OrderStatus::RESERVED)))
        .Times(1)
        .WillOnce(Return(reservedOrders));

    EXPECT_CALL(mockSampleRepo, findAll())
        .Times(1)
        .WillOnce(Return(samples));

    EXPECT_CALL(mockView, showReservedOrders(Eq(reservedOrders), _))
        .Times(1);

    auto controller = makeController();
    controller.listReservedOrders();
}

// -----------------------------------------------------------------------
// TC-11: approveOrder() — 생산량 계산 검증 (yieldRate=0.8, shortage=10)
// actualQty = ceil(10 / (0.8 * 0.9)) = ceil(10 / 0.72) = ceil(13.888...) = 14
// -----------------------------------------------------------------------
TEST_F(OrderControllerTest, ApproveOrder_StockInsufficient_DifferentYieldRate)
{
    const std::string orderId   = "ORD-011";
    const std::string sampleId  = "S-011";
    const int         stock     = 0;
    const int         quantity  = 10;   // shortage = 10 - 0 = 10
    const double      yield     = 0.8;
    const int         avgTime   = 5;
    const int         expectedActualQty = 14; // ceil(10 / (0.8 * 0.9)) = ceil(10/0.72) = 14

    Order  order  = makeOrder(orderId, sampleId, quantity, OrderStatus::RESERVED);
    Sample sample = makeSample(sampleId, stock, yield, avgTime);

    EXPECT_CALL(mockOrderRepo, findById(Eq(orderId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(order)));

    EXPECT_CALL(mockSampleRepo, findById(Eq(sampleId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(sample)));

    EXPECT_CALL(mockOrderRepo, update(_))
        .Times(1)
        .WillOnce(Return(true));

    ProductionJob capturedJob;
    EXPECT_CALL(mockProductionRepo, enqueue(_))
        .Times(1)
        .WillOnce([&capturedJob](const ProductionJob& job) {
            capturedJob = job;
        });

    EXPECT_CALL(mockView, showApprovalResult(_))
        .Times(1);

    auto controller = makeController();
    controller.approveOrder(orderId);

    EXPECT_EQ(capturedJob.actualProductionQty, expectedActualQty);
}

// -----------------------------------------------------------------------
// TC-12: rejectOrder() — sampleRepo_.update() 미호출 검증
// -----------------------------------------------------------------------
TEST_F(OrderControllerTest, RejectOrder_SampleRepoNotUpdated)
{
    const std::string orderId  = "ORD-012";
    const std::string sampleId = "S-001";
    const int         quantity = 10;

    Order order = makeOrder(orderId, sampleId, quantity, OrderStatus::RESERVED);

    EXPECT_CALL(mockOrderRepo, findById(Eq(orderId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(order)));

    EXPECT_CALL(mockOrderRepo, update(_))
        .Times(1)
        .WillOnce(Return(true));

    EXPECT_CALL(mockProductionRepo, enqueue(_))
        .Times(0);

    // rejectOrder 시 sampleRepo_.update() 호출 없음
    EXPECT_CALL(mockSampleRepo, update(_))
        .Times(0);

    EXPECT_CALL(mockView, showApprovalResult(Eq(OrderStatus::REJECTED)))
        .Times(1);

    auto controller = makeController();
    controller.rejectOrder(orderId);
}
