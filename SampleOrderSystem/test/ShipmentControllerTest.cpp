// ShipmentControllerTest.cpp
// ShipmentController 단위 테스트 (TDD — 구현 없이 먼저 작성)
// 이 시점에서 구현이 없으므로 테스트는 FAIL 상태가 정상

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Model/Repository/IOrderRepository.h"
#include "../Model/Repository/ISampleRepository.h"
#include "../View/IShipmentView.h"
#include "../Controller/ShipmentController.h"

using ::testing::Return;
using ::testing::_;
using ::testing::Eq;
using ::testing::SaveArg;

// -----------------------------------------------------------------------
// Mock 클래스
// -----------------------------------------------------------------------

class MockOrderRepository : public IOrderRepository {
public:
    MOCK_METHOD(std::string, create, (const Order&), (override));
    MOCK_METHOD(std::optional<Order>, findById, (const std::string&), (const, override));
    MOCK_METHOD(std::vector<Order>, findAll, (), (const, override));
    MOCK_METHOD(std::vector<Order>, findByStatus, (OrderStatus), (const, override));
    MOCK_METHOD(bool, update, (const Order&), (override));
};

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

class MockShipmentView : public IShipmentView {
public:
    MOCK_METHOD(void, showSubMenu, (), (override));
    MOCK_METHOD(int, getSubMenuChoice, (), (override));
    MOCK_METHOD(void, showConfirmedOrders,
        (const std::vector<Order>&, const std::vector<Sample>&), (override));
    MOCK_METHOD(void, showNoConfirmedOrders, (), (override));
    MOCK_METHOD(std::string, promptOrderIdInput, (), (override));
    MOCK_METHOD(void, showShipmentSuccess, (const std::string&, int), (override));
    MOCK_METHOD(void, showError, (const std::string&), (override));
};

// -----------------------------------------------------------------------
// 헬퍼
// -----------------------------------------------------------------------

static Order makeOrder(
    const std::string& id,
    const std::string& sampleId,
    int qty,
    OrderStatus status = OrderStatus::CONFIRMED)
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

class ShipmentControllerTest : public ::testing::Test {
protected:
    MockOrderRepository  mockOrderRepo;
    MockSampleRepository mockSampleRepo;
    MockShipmentView     mockView;

    ShipmentController makeController() {
        return ShipmentController(mockOrderRepo, mockSampleRepo, mockView);
    }
};

// -----------------------------------------------------------------------
// TC-01: processShipment() — 유효한 CONFIRMED 주문, 재고 충분
// findById → CONFIRMED order (qty=10), sample currentStock=50
// sampleRepo_.update() 1회 호출 + stock=40 검증
// orderRepo_.update() 1회 호출 + status=RELEASE 검증
// showShipmentSuccess() 1회 호출
// -----------------------------------------------------------------------
TEST_F(ShipmentControllerTest, ProcessShipment_ValidOrder_DeductsStockAndRelease)
{
    const std::string orderId  = "ORD-001";
    const std::string sampleId = "S-001";
    const int         qty      = 10;
    const int         stock    = 50;

    Order  order  = makeOrder(orderId, sampleId, qty, OrderStatus::CONFIRMED);
    Sample sample = makeSample(sampleId, stock);

    EXPECT_CALL(mockView, promptOrderIdInput())
        .Times(1)
        .WillOnce(Return(orderId));

    EXPECT_CALL(mockOrderRepo, findById(Eq(orderId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(order)));

    EXPECT_CALL(mockSampleRepo, findById(Eq(sampleId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(sample)));

    Sample updatedSample;
    EXPECT_CALL(mockSampleRepo, update(_))
        .Times(1)
        .WillOnce([&updatedSample](const Sample& s) {
            updatedSample = s;
            return true;
        });

    Order updatedOrder;
    EXPECT_CALL(mockOrderRepo, update(_))
        .Times(1)
        .WillOnce([&updatedOrder](const Order& o) {
            updatedOrder = o;
            return true;
        });

    EXPECT_CALL(mockView, showShipmentSuccess(Eq(orderId), Eq(qty)))
        .Times(1);

    EXPECT_CALL(mockView, showError(_))
        .Times(0);

    auto controller = makeController();
    controller.processShipment();

    EXPECT_EQ(updatedSample.currentStock, stock - qty);  // 50 - 10 = 40
    EXPECT_EQ(updatedOrder.status, OrderStatus::RELEASE);
}

// -----------------------------------------------------------------------
// TC-02: processShipment() — 주문 ID 미존재 → showError() 1회
// sampleRepo_.update() 0회, orderRepo_.update() 0회
// -----------------------------------------------------------------------
TEST_F(ShipmentControllerTest, ProcessShipment_OrderNotFound_ShowsError)
{
    const std::string orderId = "ORD-UNKNOWN";

    EXPECT_CALL(mockView, promptOrderIdInput())
        .Times(1)
        .WillOnce(Return(orderId));

    EXPECT_CALL(mockOrderRepo, findById(Eq(orderId)))
        .Times(1)
        .WillOnce(Return(std::nullopt));

    EXPECT_CALL(mockSampleRepo, update(_))
        .Times(0);

    EXPECT_CALL(mockOrderRepo, update(_))
        .Times(0);

    EXPECT_CALL(mockView, showError(_))
        .Times(1);

    EXPECT_CALL(mockView, showShipmentSuccess(_, _))
        .Times(0);

    auto controller = makeController();
    controller.processShipment();
}

// -----------------------------------------------------------------------
// TC-03: processShipment() — 재고 부족 (currentStock=5, qty=10)
// showError("재고 부족으로 출고 불가") 1회
// sampleRepo_.update() 0회, orderRepo_.update() 0회
// -----------------------------------------------------------------------
TEST_F(ShipmentControllerTest, ProcessShipment_InsufficientStock_ShowsError)
{
    const std::string orderId  = "ORD-002";
    const std::string sampleId = "S-002";
    const int         qty      = 10;
    const int         stock    = 5;  // stock < qty

    Order  order  = makeOrder(orderId, sampleId, qty, OrderStatus::CONFIRMED);
    Sample sample = makeSample(sampleId, stock);

    EXPECT_CALL(mockView, promptOrderIdInput())
        .Times(1)
        .WillOnce(Return(orderId));

    EXPECT_CALL(mockOrderRepo, findById(Eq(orderId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(order)));

    EXPECT_CALL(mockSampleRepo, findById(Eq(sampleId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(sample)));

    EXPECT_CALL(mockSampleRepo, update(_))
        .Times(0);

    EXPECT_CALL(mockOrderRepo, update(_))
        .Times(0);

    EXPECT_CALL(mockView, showError(_))
        .Times(1);

    EXPECT_CALL(mockView, showShipmentSuccess(_, _))
        .Times(0);

    auto controller = makeController();
    controller.processShipment();
}

// -----------------------------------------------------------------------
// TC-04: processShipment() — 경계값: currentStock == qty (stock=10, qty=10)
// 출고 성공 → stock=0, status=RELEASE
// -----------------------------------------------------------------------
TEST_F(ShipmentControllerTest, ProcessShipment_StockExactlyEqual_Succeeds)
{
    const std::string orderId  = "ORD-003";
    const std::string sampleId = "S-003";
    const int         qty      = 10;
    const int         stock    = 10;  // stock == qty → 성공

    Order  order  = makeOrder(orderId, sampleId, qty, OrderStatus::CONFIRMED);
    Sample sample = makeSample(sampleId, stock);

    EXPECT_CALL(mockView, promptOrderIdInput())
        .Times(1)
        .WillOnce(Return(orderId));

    EXPECT_CALL(mockOrderRepo, findById(Eq(orderId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(order)));

    EXPECT_CALL(mockSampleRepo, findById(Eq(sampleId)))
        .Times(1)
        .WillOnce(Return(std::make_optional(sample)));

    Sample updatedSample;
    EXPECT_CALL(mockSampleRepo, update(_))
        .Times(1)
        .WillOnce([&updatedSample](const Sample& s) {
            updatedSample = s;
            return true;
        });

    Order updatedOrder;
    EXPECT_CALL(mockOrderRepo, update(_))
        .Times(1)
        .WillOnce([&updatedOrder](const Order& o) {
            updatedOrder = o;
            return true;
        });

    EXPECT_CALL(mockView, showShipmentSuccess(Eq(orderId), Eq(qty)))
        .Times(1);

    EXPECT_CALL(mockView, showError(_))
        .Times(0);

    auto controller = makeController();
    controller.processShipment();

    EXPECT_EQ(updatedSample.currentStock, 0);              // 10 - 10 = 0
    EXPECT_EQ(updatedOrder.status, OrderStatus::RELEASE);
}

// -----------------------------------------------------------------------
// TC-05: listConfirmedOrders() — CONFIRMED 주문 2개 존재
// findByStatus(CONFIRMED) 1회, findAll() 1회
// showConfirmedOrders() 1회, showNoConfirmedOrders() 0회
// -----------------------------------------------------------------------
TEST_F(ShipmentControllerTest, ListConfirmedOrders_HasOrders_CallsShowConfirmedOrders)
{
    std::vector<Order> confirmedOrders = {
        makeOrder("ORD-010", "S-001", 10, OrderStatus::CONFIRMED),
        makeOrder("ORD-011", "S-002", 20, OrderStatus::CONFIRMED),
    };

    std::vector<Sample> allSamples = {
        makeSample("S-001", 100),
        makeSample("S-002", 50),
    };

    EXPECT_CALL(mockOrderRepo, findByStatus(Eq(OrderStatus::CONFIRMED)))
        .Times(1)
        .WillOnce(Return(confirmedOrders));

    EXPECT_CALL(mockSampleRepo, findAll())
        .Times(1)
        .WillOnce(Return(allSamples));

    EXPECT_CALL(mockView, showConfirmedOrders(Eq(confirmedOrders), _))
        .Times(1);

    EXPECT_CALL(mockView, showNoConfirmedOrders())
        .Times(0);

    auto controller = makeController();
    controller.listConfirmedOrders();
}

// -----------------------------------------------------------------------
// TC-06: listConfirmedOrders() — CONFIRMED 주문 없음
// findByStatus(CONFIRMED) → 빈 벡터
// showNoConfirmedOrders() 1회, showConfirmedOrders() 0회
// -----------------------------------------------------------------------
TEST_F(ShipmentControllerTest, ListConfirmedOrders_NoOrders_CallsShowNoConfirmedOrders)
{
    EXPECT_CALL(mockOrderRepo, findByStatus(Eq(OrderStatus::CONFIRMED)))
        .Times(1)
        .WillOnce(Return(std::vector<Order>{}));

    EXPECT_CALL(mockView, showNoConfirmedOrders())
        .Times(1);

    EXPECT_CALL(mockView, showConfirmedOrders(_, _))
        .Times(0);

    EXPECT_CALL(mockSampleRepo, findAll())
        .Times(0);

    auto controller = makeController();
    controller.listConfirmedOrders();
}
