// MonitoringControllerTest.cpp
// MonitoringController 단위 테스트 (TDD — 구현 없이 먼저 작성)
// 이 시점에서 구현이 없으므로 테스트는 FAIL 상태가 정상

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Model/Repository/IOrderRepository.h"
#include "../Model/Repository/ISampleRepository.h"
#include "../View/IMonitoringView.h"
#include "../Controller/MonitoringController.h"

using ::testing::Return;
using ::testing::_;

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
    MOCK_METHOD(void, clearAll, (), (override));
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
    MOCK_METHOD(void, clearAll, (), (override));
};

class MockMonitoringView : public IMonitoringView {
public:
    MOCK_METHOD(void, showSubMenu, (), (override));
    MOCK_METHOD(int, getSubMenuChoice, (), (override));
    MOCK_METHOD(void, showOrderStats,
        (const std::vector<Order>&,
         const std::vector<Order>&,
         const std::vector<Order>&,
         const std::vector<Order>&),
        (override));
    MOCK_METHOD(void, showStockStatus,
        (const std::vector<Sample>&,
         const std::vector<StockStatus>&),
        (override));
};

// -----------------------------------------------------------------------
// 헬퍼
// -----------------------------------------------------------------------

static Order makeOrder(const std::string& id, const std::string& sampleId,
                       int qty, OrderStatus status)
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

static Sample makeSample(const std::string& id, int stock)
{
    Sample s;
    s.id                = id;
    s.name              = "TestSample";
    s.avgProductionTime = 10;
    s.yieldRate         = 0.9;
    s.currentStock      = stock;
    return s;
}

// -----------------------------------------------------------------------
// Fixture
// -----------------------------------------------------------------------

class MonitoringControllerTest : public ::testing::Test {
protected:
    MockOrderRepository  mockOrderRepo;
    MockSampleRepository mockSampleRepo;
    MockMonitoringView   mockView;

    MonitoringController makeController() {
        return MonitoringController(mockOrderRepo, mockSampleRepo, mockView);
    }
};

// -----------------------------------------------------------------------
// TC-01: ShowOrderStats_RejectsExcluded_CorrectGrouping (BR-12)
// RESERVED 2개, PRODUCING 1개, CONFIRMED 3개, RELEASE 1개, REJECTED 2개
// showOrderStats() 에 전달된 각 벡터 크기: reserved=2, producing=1, confirmed=3, released=1
// REJECTED 는 어느 벡터에도 없음
// -----------------------------------------------------------------------
TEST_F(MonitoringControllerTest, ShowOrderStats_RejectsExcluded_CorrectGrouping)
{
    std::vector<Order> allOrders = {
        makeOrder("ORD-001", "S-001", 10, OrderStatus::RESERVED),
        makeOrder("ORD-002", "S-001", 10, OrderStatus::RESERVED),
        makeOrder("ORD-003", "S-001", 10, OrderStatus::PRODUCING),
        makeOrder("ORD-004", "S-001", 10, OrderStatus::CONFIRMED),
        makeOrder("ORD-005", "S-001", 10, OrderStatus::CONFIRMED),
        makeOrder("ORD-006", "S-001", 10, OrderStatus::CONFIRMED),
        makeOrder("ORD-007", "S-001", 10, OrderStatus::RELEASE),
        makeOrder("ORD-008", "S-001", 10, OrderStatus::REJECTED),
        makeOrder("ORD-009", "S-001", 10, OrderStatus::REJECTED),
    };

    EXPECT_CALL(mockOrderRepo, findAll())
        .Times(1)
        .WillOnce(Return(allOrders));

    std::vector<Order> capturedReserved;
    std::vector<Order> capturedProducing;
    std::vector<Order> capturedConfirmed;
    std::vector<Order> capturedReleased;

    EXPECT_CALL(mockView, showOrderStats(_, _, _, _))
        .Times(1)
        .WillOnce([&](const std::vector<Order>& reserved,
                      const std::vector<Order>& producing,
                      const std::vector<Order>& confirmed,
                      const std::vector<Order>& released) {
            capturedReserved  = reserved;
            capturedProducing = producing;
            capturedConfirmed = confirmed;
            capturedReleased  = released;
        });

    auto controller = makeController();
    controller.showOrderStats();

    EXPECT_EQ(capturedReserved.size(),  2u);
    EXPECT_EQ(capturedProducing.size(), 1u);
    EXPECT_EQ(capturedConfirmed.size(), 3u);
    EXPECT_EQ(capturedReleased.size(),  1u);

    // REJECTED 는 어느 벡터에도 없음
    auto hasRejected = [](const std::vector<Order>& v) {
        for (const auto& o : v)
            if (o.status == OrderStatus::REJECTED) return true;
        return false;
    };
    EXPECT_FALSE(hasRejected(capturedReserved));
    EXPECT_FALSE(hasRejected(capturedProducing));
    EXPECT_FALSE(hasRejected(capturedConfirmed));
    EXPECT_FALSE(hasRejected(capturedReleased));
}

// -----------------------------------------------------------------------
// TC-02: ShowStockStatus_SurplusStock
// sample currentStock=100, 유효 주문 합계=30
// statusList[0] == StockStatus::SURPLUS
// -----------------------------------------------------------------------
TEST_F(MonitoringControllerTest, ShowStockStatus_SurplusStock)
{
    const std::string sampleId = "S-001";

    std::vector<Sample> samples = { makeSample(sampleId, 100) };

    std::vector<Order> allOrders = {
        makeOrder("ORD-001", sampleId, 30, OrderStatus::RESERVED),
    };

    EXPECT_CALL(mockSampleRepo, findAll())
        .Times(1)
        .WillOnce(Return(samples));

    EXPECT_CALL(mockOrderRepo, findAll())
        .Times(1)
        .WillOnce(Return(allOrders));

    std::vector<StockStatus> capturedStatusList;
    EXPECT_CALL(mockView, showStockStatus(_, _))
        .Times(1)
        .WillOnce([&](const std::vector<Sample>&,
                      const std::vector<StockStatus>& statusList) {
            capturedStatusList = statusList;
        });

    auto controller = makeController();
    controller.showStockStatus();

    ASSERT_EQ(capturedStatusList.size(), 1u);
    EXPECT_EQ(capturedStatusList[0], StockStatus::SURPLUS);
}

// -----------------------------------------------------------------------
// TC-03: ShowStockStatus_ShortageStock_StockLessOrEqualToSum
// currentStock=30, 유효 합계=30 → SHORTAGE
// -----------------------------------------------------------------------
TEST_F(MonitoringControllerTest, ShowStockStatus_ShortageStock_StockLessOrEqualToSum)
{
    const std::string sampleId = "S-001";

    std::vector<Sample> samples = { makeSample(sampleId, 30) };

    std::vector<Order> allOrders = {
        makeOrder("ORD-001", sampleId, 30, OrderStatus::CONFIRMED),
    };

    EXPECT_CALL(mockSampleRepo, findAll())
        .Times(1)
        .WillOnce(Return(samples));

    EXPECT_CALL(mockOrderRepo, findAll())
        .Times(1)
        .WillOnce(Return(allOrders));

    std::vector<StockStatus> capturedStatusList;
    EXPECT_CALL(mockView, showStockStatus(_, _))
        .Times(1)
        .WillOnce([&](const std::vector<Sample>&,
                      const std::vector<StockStatus>& statusList) {
            capturedStatusList = statusList;
        });

    auto controller = makeController();
    controller.showStockStatus();

    ASSERT_EQ(capturedStatusList.size(), 1u);
    EXPECT_EQ(capturedStatusList[0], StockStatus::SHORTAGE);
}

// -----------------------------------------------------------------------
// TC-04: ShowStockStatus_ShortageStock_StockLessThanSum
// currentStock=20, 유효 합계=30 → SHORTAGE
// -----------------------------------------------------------------------
TEST_F(MonitoringControllerTest, ShowStockStatus_ShortageStock_StockLessThanSum)
{
    const std::string sampleId = "S-001";

    std::vector<Sample> samples = { makeSample(sampleId, 20) };

    std::vector<Order> allOrders = {
        makeOrder("ORD-001", sampleId, 15, OrderStatus::RESERVED),
        makeOrder("ORD-002", sampleId, 15, OrderStatus::PRODUCING),
    };

    EXPECT_CALL(mockSampleRepo, findAll())
        .Times(1)
        .WillOnce(Return(samples));

    EXPECT_CALL(mockOrderRepo, findAll())
        .Times(1)
        .WillOnce(Return(allOrders));

    std::vector<StockStatus> capturedStatusList;
    EXPECT_CALL(mockView, showStockStatus(_, _))
        .Times(1)
        .WillOnce([&](const std::vector<Sample>&,
                      const std::vector<StockStatus>& statusList) {
            capturedStatusList = statusList;
        });

    auto controller = makeController();
    controller.showStockStatus();

    ASSERT_EQ(capturedStatusList.size(), 1u);
    EXPECT_EQ(capturedStatusList[0], StockStatus::SHORTAGE);
}

// -----------------------------------------------------------------------
// TC-05: ShowStockStatus_DepletedStock
// currentStock=0 (유효 합계 무관) → DEPLETED
// -----------------------------------------------------------------------
TEST_F(MonitoringControllerTest, ShowStockStatus_DepletedStock)
{
    const std::string sampleId = "S-001";

    std::vector<Sample> samples = { makeSample(sampleId, 0) };

    std::vector<Order> allOrders = {
        makeOrder("ORD-001", sampleId, 10, OrderStatus::RESERVED),
    };

    EXPECT_CALL(mockSampleRepo, findAll())
        .Times(1)
        .WillOnce(Return(samples));

    EXPECT_CALL(mockOrderRepo, findAll())
        .Times(1)
        .WillOnce(Return(allOrders));

    std::vector<StockStatus> capturedStatusList;
    EXPECT_CALL(mockView, showStockStatus(_, _))
        .Times(1)
        .WillOnce([&](const std::vector<Sample>&,
                      const std::vector<StockStatus>& statusList) {
            capturedStatusList = statusList;
        });

    auto controller = makeController();
    controller.showStockStatus();

    ASSERT_EQ(capturedStatusList.size(), 1u);
    EXPECT_EQ(capturedStatusList[0], StockStatus::DEPLETED);
}

// -----------------------------------------------------------------------
// TC-06: ShowStockStatus_NoValidOrders_Surplus
// 유효 주문 없음, currentStock=50 → SURPLUS
// -----------------------------------------------------------------------
TEST_F(MonitoringControllerTest, ShowStockStatus_NoValidOrders_Surplus)
{
    const std::string sampleId = "S-001";

    std::vector<Sample> samples = { makeSample(sampleId, 50) };

    // 유효 주문 없음
    std::vector<Order> allOrders = {};

    EXPECT_CALL(mockSampleRepo, findAll())
        .Times(1)
        .WillOnce(Return(samples));

    EXPECT_CALL(mockOrderRepo, findAll())
        .Times(1)
        .WillOnce(Return(allOrders));

    std::vector<StockStatus> capturedStatusList;
    EXPECT_CALL(mockView, showStockStatus(_, _))
        .Times(1)
        .WillOnce([&](const std::vector<Sample>&,
                      const std::vector<StockStatus>& statusList) {
            capturedStatusList = statusList;
        });

    auto controller = makeController();
    controller.showStockStatus();

    ASSERT_EQ(capturedStatusList.size(), 1u);
    EXPECT_EQ(capturedStatusList[0], StockStatus::SURPLUS);
}

// -----------------------------------------------------------------------
// TC-07: ShowStockStatus_ReleaseOrdersExcluded
// RELEASE 주문 qty=50이 있어도 유효 합계에서 제외
// currentStock=10, RELEASE qty=50 → 합계=0 → SURPLUS
// -----------------------------------------------------------------------
TEST_F(MonitoringControllerTest, ShowStockStatus_ReleaseOrdersExcluded)
{
    const std::string sampleId = "S-001";

    std::vector<Sample> samples = { makeSample(sampleId, 10) };

    std::vector<Order> allOrders = {
        makeOrder("ORD-001", sampleId, 50, OrderStatus::RELEASE),
    };

    EXPECT_CALL(mockSampleRepo, findAll())
        .Times(1)
        .WillOnce(Return(samples));

    EXPECT_CALL(mockOrderRepo, findAll())
        .Times(1)
        .WillOnce(Return(allOrders));

    std::vector<StockStatus> capturedStatusList;
    EXPECT_CALL(mockView, showStockStatus(_, _))
        .Times(1)
        .WillOnce([&](const std::vector<Sample>&,
                      const std::vector<StockStatus>& statusList) {
            capturedStatusList = statusList;
        });

    auto controller = makeController();
    controller.showStockStatus();

    ASSERT_EQ(capturedStatusList.size(), 1u);
    // RELEASE 주문은 유효 합계에서 제외 → 합계=0 → currentStock(10) > 0 → SURPLUS
    EXPECT_EQ(capturedStatusList[0], StockStatus::SURPLUS);
}

// -----------------------------------------------------------------------
// TC-08: ShowStockStatus_RejectedOrdersExcluded
// REJECTED 주문 qty=50이 있어도 합계에서 제외
// currentStock=10 → 합계=0 → SURPLUS
// -----------------------------------------------------------------------
TEST_F(MonitoringControllerTest, ShowStockStatus_RejectedOrdersExcluded)
{
    const std::string sampleId = "S-001";

    std::vector<Sample> samples = { makeSample(sampleId, 10) };

    std::vector<Order> allOrders = {
        makeOrder("ORD-001", sampleId, 50, OrderStatus::REJECTED),
    };

    EXPECT_CALL(mockSampleRepo, findAll())
        .Times(1)
        .WillOnce(Return(samples));

    EXPECT_CALL(mockOrderRepo, findAll())
        .Times(1)
        .WillOnce(Return(allOrders));

    std::vector<StockStatus> capturedStatusList;
    EXPECT_CALL(mockView, showStockStatus(_, _))
        .Times(1)
        .WillOnce([&](const std::vector<Sample>&,
                      const std::vector<StockStatus>& statusList) {
            capturedStatusList = statusList;
        });

    auto controller = makeController();
    controller.showStockStatus();

    ASSERT_EQ(capturedStatusList.size(), 1u);
    // REJECTED 주문은 유효 합계에서 제외 → 합계=0 → currentStock(10) > 0 → SURPLUS
    EXPECT_EQ(capturedStatusList[0], StockStatus::SURPLUS);
}

// -----------------------------------------------------------------------
// TC-09: ShowOrderStats_AllEmpty_CallsWithEmptyVectors
// 주문 없을 때 빈 벡터 4개로 showOrderStats 호출
// -----------------------------------------------------------------------
TEST_F(MonitoringControllerTest, ShowOrderStats_AllEmpty_CallsWithEmptyVectors)
{
    EXPECT_CALL(mockOrderRepo, findAll())
        .Times(1)
        .WillOnce(Return(std::vector<Order>{}));

    std::vector<Order> capturedReserved;
    std::vector<Order> capturedProducing;
    std::vector<Order> capturedConfirmed;
    std::vector<Order> capturedReleased;

    EXPECT_CALL(mockView, showOrderStats(_, _, _, _))
        .Times(1)
        .WillOnce([&](const std::vector<Order>& reserved,
                      const std::vector<Order>& producing,
                      const std::vector<Order>& confirmed,
                      const std::vector<Order>& released) {
            capturedReserved  = reserved;
            capturedProducing = producing;
            capturedConfirmed = confirmed;
            capturedReleased  = released;
        });

    auto controller = makeController();
    controller.showOrderStats();

    EXPECT_EQ(capturedReserved.size(),  0u);
    EXPECT_EQ(capturedProducing.size(), 0u);
    EXPECT_EQ(capturedConfirmed.size(), 0u);
    EXPECT_EQ(capturedReleased.size(),  0u);
}

// -----------------------------------------------------------------------
// TC-10: ShowStockStatus_NoSamples_CallsShowWithEmptyVectors
// 시료 없을 때 빈 벡터 2개로 showStockStatus 호출
// -----------------------------------------------------------------------
TEST_F(MonitoringControllerTest, ShowStockStatus_NoSamples_CallsShowWithEmptyVectors)
{
    EXPECT_CALL(mockSampleRepo, findAll())
        .Times(1)
        .WillOnce(Return(std::vector<Sample>{}));

    EXPECT_CALL(mockOrderRepo, findAll())
        .Times(1)
        .WillOnce(Return(std::vector<Order>{}));

    std::vector<Sample>      capturedSamples;
    std::vector<StockStatus> capturedStatusList;

    EXPECT_CALL(mockView, showStockStatus(_, _))
        .Times(1)
        .WillOnce([&](const std::vector<Sample>& samples,
                      const std::vector<StockStatus>& statusList) {
            capturedSamples    = samples;
            capturedStatusList = statusList;
        });

    auto controller = makeController();
    controller.showStockStatus();

    EXPECT_EQ(capturedSamples.size(),    0u);
    EXPECT_EQ(capturedStatusList.size(), 0u);
}
