// ProductionServiceTest.cpp
// ProductionService 단위 테스트 (TDD 1단계 — 구현 없이 작성)
// 이 시점에서 구현이 없으므로 테스트는 FAIL 상태가 정상

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Model/Repository/ISampleRepository.h"
#include "../Model/Repository/IOrderRepository.h"
#include "../Model/Repository/IProductionRepository.h"
#include "../Model/Service/ITimeProvider.h"
#include "../Model/Service/ProductionService.h"

using ::testing::_;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::NiceMock;
using ::testing::InSequence;
using ::testing::AtLeast;

// -----------------------------------------------------------------------
// Mock 클래스
// -----------------------------------------------------------------------

class MockSampleRepository : public ISampleRepository {
public:
    MOCK_METHOD(bool, save,   (const Sample&),              (override));
    MOCK_METHOD(bool, update, (const Sample&),              (override));
    MOCK_METHOD(std::optional<Sample>, findById,    (const std::string&), (const, override));
    MOCK_METHOD(std::vector<Sample>,   findAll,     (),                   (const, override));
    MOCK_METHOD(std::vector<Sample>,   searchByName,(const std::string&), (const, override));
    MOCK_METHOD(bool, existsId,  (const std::string&), (const, override));
    MOCK_METHOD(bool, existsName,(const std::string&), (const, override));
    MOCK_METHOD(void, clearAll, (), (override));
};

class MockOrderRepository : public IOrderRepository {
public:
    MOCK_METHOD(std::string,           create,       (const Order&),      (override));
    MOCK_METHOD(std::optional<Order>,  findById,     (const std::string&),(const, override));
    MOCK_METHOD(std::vector<Order>,    findAll,      (),                  (const, override));
    MOCK_METHOD(std::vector<Order>,    findByStatus, (OrderStatus),       (const, override));
    MOCK_METHOD(bool,                  update,       (const Order&),      (override));
    MOCK_METHOD(void,                  clearAll,     (),                  (override));
};

class MockProductionRepository : public IProductionRepository {
public:
    MOCK_METHOD(ProductionState, getState,  (),                        (const, override));
    MOCK_METHOD(void,            setState,  (const ProductionState&),  (override));
    MOCK_METHOD(void,            enqueue,   (const ProductionJob&),    (override));
    MOCK_METHOD(void,            clearAll,  (),                        (override));
};

class MockTimeProviderGMock : public ITimeProvider {
public:
    MOCK_METHOD(time_t,      now,       (), (const, override));
    MOCK_METHOD(std::string, nowIso8601,(), (const, override));
};

// -----------------------------------------------------------------------
// 헬퍼
// -----------------------------------------------------------------------

static ProductionJob makeJob(
    const std::string& orderId  = "ORD-001",
    const std::string& sampleId = "S-001",
    int    shortage             = 5,
    int    actualQty            = 7,
    int    totalMin             = 35,
    int64_t startTime           = 0)
{
    ProductionJob j;
    j.orderId                = orderId;
    j.sampleId               = sampleId;
    j.shortage               = shortage;
    j.actualProductionQty    = actualQty;
    j.totalProductionTimeMin = totalMin;
    j.startTimeUnix          = startTime;
    return j;
}

static Order makeOrder(
    const std::string& id,
    const std::string& sampleId,
    int qty,
    OrderStatus status = OrderStatus::PRODUCING)
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

class ProductionServiceTest : public ::testing::Test {
protected:
    MockSampleRepository     mockSample;
    MockOrderRepository      mockOrder;
    MockProductionRepository mockProduction;
    MockTimeProviderGMock    mockTime;

    ProductionService makeService() {
        return ProductionService(mockSample, mockOrder, mockProduction, mockTime);
    }
};

// -----------------------------------------------------------------------
// TC-01: activeJob 없음 → 아무것도 하지 않음
// getState() → activeJob=nullopt
// sampleRepo_.update(), orderRepo_.update() 미호출
// -----------------------------------------------------------------------
TEST_F(ProductionServiceTest, CheckAndComplete_NoActiveJob_DoesNothing)
{
    ProductionState emptyState;
    emptyState.activeJob = std::nullopt;

    ON_CALL(mockProduction, getState())
        .WillByDefault(Return(emptyState));

    EXPECT_CALL(mockSample, update(_))
        .Times(0);
    EXPECT_CALL(mockOrder, update(_))
        .Times(0);

    auto svc = makeService();
    svc.checkAndComplete();
}

// -----------------------------------------------------------------------
// TC-02: activeJob 있지만 미완료 → 아무것도 하지 않음
// startTimeUnix=0, totalProductionTimeMin=60
// now()=3000 → elapsed=3000 < 3600 → 미완료
// -----------------------------------------------------------------------
TEST_F(ProductionServiceTest, CheckAndComplete_ActiveJobNotDone_DoesNothing)
{
    ProductionJob job = makeJob("ORD-001", "S-001", 5, 7, 60, 0);

    ProductionState state;
    state.activeJob = job;

    ON_CALL(mockProduction, getState())
        .WillByDefault(Return(state));
    ON_CALL(mockTime, now())
        .WillByDefault(Return(static_cast<time_t>(3000)));

    EXPECT_CALL(mockSample, update(_))
        .Times(0);
    EXPECT_CALL(mockOrder, update(_))
        .Times(0);

    auto svc = makeService();
    svc.checkAndComplete();
}

// -----------------------------------------------------------------------
// TC-03: activeJob 완료 → sampleRepo_.update() 1회 호출, stock += actualProductionQty 검증
// startTimeUnix=0, totalProductionTimeMin=10
// now()=700 → elapsed=700 > 600 → 완료
// -----------------------------------------------------------------------
TEST_F(ProductionServiceTest, CheckAndComplete_ActiveJobDone_StockIncreased)
{
    const std::string sampleId  = "S-001";
    const int         initStock = 10;
    const int         actualQty = 7;

    ProductionJob job = makeJob("ORD-001", sampleId, 5, actualQty, 10, 0);

    ProductionState state;
    state.activeJob = job;

    Sample sample = makeSample(sampleId, initStock);
    Order  order  = makeOrder("ORD-001", sampleId, 20, OrderStatus::PRODUCING);

    ON_CALL(mockProduction, getState())
        .WillByDefault(Return(state));
    ON_CALL(mockTime, now())
        .WillByDefault(Return(static_cast<time_t>(700)));
    ON_CALL(mockSample, findById(sampleId))
        .WillByDefault(Return(std::make_optional(sample)));
    ON_CALL(mockOrder, findById("ORD-001"))
        .WillByDefault(Return(std::make_optional(order)));
    ON_CALL(mockOrder, update(_))
        .WillByDefault(Return(true));
    ON_CALL(mockProduction, setState(_))
        .WillByDefault(Return());

    Sample capturedSample;
    EXPECT_CALL(mockSample, update(_))
        .Times(1)
        .WillOnce([&capturedSample](const Sample& s) {
            capturedSample = s;
            return true;
        });

    auto svc = makeService();
    svc.checkAndComplete();

    EXPECT_EQ(capturedSample.currentStock, initStock + actualQty);
}

// -----------------------------------------------------------------------
// TC-04: activeJob 완료 → orderRepo_.update() 1회 호출, order.status == CONFIRMED 검증
// -----------------------------------------------------------------------
TEST_F(ProductionServiceTest, CheckAndComplete_ActiveJobDone_OrderBecomesConfirmed)
{
    const std::string sampleId = "S-001";
    const std::string orderId  = "ORD-001";

    ProductionJob job = makeJob(orderId, sampleId, 5, 7, 10, 0);

    ProductionState state;
    state.activeJob = job;

    Sample sample = makeSample(sampleId, 10);
    Order  order  = makeOrder(orderId, sampleId, 20, OrderStatus::PRODUCING);

    ON_CALL(mockProduction, getState())
        .WillByDefault(Return(state));
    ON_CALL(mockTime, now())
        .WillByDefault(Return(static_cast<time_t>(700)));
    ON_CALL(mockSample, findById(sampleId))
        .WillByDefault(Return(std::make_optional(sample)));
    ON_CALL(mockOrder, findById(orderId))
        .WillByDefault(Return(std::make_optional(order)));
    ON_CALL(mockSample, update(_))
        .WillByDefault(Return(true));
    ON_CALL(mockProduction, setState(_))
        .WillByDefault(Return());

    Order capturedOrder;
    EXPECT_CALL(mockOrder, update(_))
        .Times(1)
        .WillOnce([&capturedOrder](const Order& o) {
            capturedOrder = o;
            return true;
        });

    auto svc = makeService();
    svc.checkAndComplete();

    EXPECT_EQ(capturedOrder.status, OrderStatus::CONFIRMED);
}

// -----------------------------------------------------------------------
// TC-05: activeJob 완료 후 setState() 호출 시 activeJob=nullopt 검증
// -----------------------------------------------------------------------
TEST_F(ProductionServiceTest, CheckAndComplete_ActiveJobDone_ActiveJobCleared)
{
    const std::string sampleId = "S-001";
    const std::string orderId  = "ORD-001";

    ProductionJob job = makeJob(orderId, sampleId, 5, 7, 10, 0);

    ProductionState state;
    state.activeJob = job;

    Sample sample = makeSample(sampleId, 10);
    Order  order  = makeOrder(orderId, sampleId, 20, OrderStatus::PRODUCING);

    ON_CALL(mockProduction, getState())
        .WillByDefault(Return(state));
    ON_CALL(mockTime, now())
        .WillByDefault(Return(static_cast<time_t>(700)));
    ON_CALL(mockSample, findById(sampleId))
        .WillByDefault(Return(std::make_optional(sample)));
    ON_CALL(mockOrder, findById(orderId))
        .WillByDefault(Return(std::make_optional(order)));
    ON_CALL(mockSample, update(_))
        .WillByDefault(Return(true));
    ON_CALL(mockOrder, update(_))
        .WillByDefault(Return(true));

    ProductionState capturedState;
    EXPECT_CALL(mockProduction, setState(_))
        .Times(1)
        .WillOnce([&capturedState](const ProductionState& s) {
            capturedState = s;
        });

    auto svc = makeService();
    svc.checkAndComplete();

    EXPECT_FALSE(capturedState.activeJob.has_value());
}

// -----------------------------------------------------------------------
// TC-06: 완료 후 queue에 다음 job 있음 → startNextJob() 호출
// setState() 호출 시 새 activeJob.orderId = queue의 첫 번째 orderId 검증
// 새 activeJob.startTimeUnix = timeProvider_.now() 값 검증
// getState() 2회 호출: 1회차=초기상태, 2회차=새 activeJob(미완료) → 루프 종료
// -----------------------------------------------------------------------
TEST_F(ProductionServiceTest, CheckAndComplete_QueueHasNext_StartsNextJob)
{
    const std::string sampleId   = "S-001";
    const std::string orderId1   = "ORD-001";  // 완료될 active job
    const std::string orderId2   = "ORD-002";  // 다음으로 시작될 queue의 첫 번째 job
    const time_t      fixedNow   = static_cast<time_t>(700);

    // 현재 active job: 10분짜리, 0초에 시작 → 700초 경과 → 완료
    ProductionJob activeJob = makeJob(orderId1, sampleId, 5, 7, 10, 0);
    // queue에 있는 다음 job (20분짜리, fixedNow에 시작되므로 elapsed=0 → 미완료)
    ProductionJob nextJob   = makeJob(orderId2, sampleId, 3, 4, 20, 0);

    ProductionState initialState;
    initialState.activeJob = activeJob;
    initialState.queue.push_back(nextJob);

    // 두 번째 getState(): startNextJob 이후, 새 activeJob은 방금 시작됨 → 미완료
    ProductionState secondState;
    ProductionJob runningNext = nextJob;
    runningNext.startTimeUnix = static_cast<int64_t>(fixedNow);
    secondState.activeJob     = runningNext;

    Sample sample = makeSample(sampleId, 10);
    Order  order1 = makeOrder(orderId1, sampleId, 20, OrderStatus::PRODUCING);

    // getState()는 순서대로: 초기 상태 → 미완료 상태 (루프 2회)
    {
        InSequence seq;
        EXPECT_CALL(mockProduction, getState())
            .Times(2)
            .WillOnce(Return(initialState))
            .WillOnce(Return(secondState));
    }

    ON_CALL(mockTime, now())
        .WillByDefault(Return(fixedNow));
    ON_CALL(mockSample, findById(sampleId))
        .WillByDefault(Return(std::make_optional(sample)));
    ON_CALL(mockOrder, findById(orderId1))
        .WillByDefault(Return(std::make_optional(order1)));
    ON_CALL(mockSample, update(_))
        .WillByDefault(Return(true));
    ON_CALL(mockOrder, update(_))
        .WillByDefault(Return(true));

    // setState()는 1회 호출: startNextJob 직후
    ProductionState capturedState;
    EXPECT_CALL(mockProduction, setState(_))
        .Times(1)
        .WillOnce([&capturedState](const ProductionState& s) {
            capturedState = s;
        });

    auto svc = makeService();
    svc.checkAndComplete();

    // setState에서 새 activeJob이 orderId2로 설정됐는지 확인
    ASSERT_TRUE(capturedState.activeJob.has_value())
        << "setState()에 activeJob이 설정되어야 한다.";
    EXPECT_EQ(capturedState.activeJob->orderId, orderId2)
        << "queue의 다음 job(ORD-002)이 activeJob으로 시작되어야 한다.";
    EXPECT_EQ(capturedState.activeJob->startTimeUnix, static_cast<int64_t>(fixedNow))
        << "새 activeJob의 startTimeUnix는 now() 값이어야 한다.";
}

// -----------------------------------------------------------------------
// TC-07: 연쇄 완료 처리 (BR-17) — activeJob + queue 1개, 둘 다 충분히 경과됨
// 루프 흐름:
//   1회차 getState: job1 active, job2 queue → job1 완료 → startNextJob → setState → 루프
//   2회차 getState: job2 active(완료) → completeCurrentJob → queue 비어있음 → setState → return
// getState() 2회, orderRepo_.update() 2회, sampleRepo_.update() 2회 검증
// -----------------------------------------------------------------------
TEST_F(ProductionServiceTest, CheckAndComplete_ChainedJobs_BothCompleted)
{
    const std::string sampleId1 = "S-001";
    const std::string sampleId2 = "S-002";
    const std::string orderId1  = "ORD-001";
    const std::string orderId2  = "ORD-002";
    // 매우 큰 now() → 두 job 모두 경과 시간 초과
    const time_t bigNow = static_cast<time_t>(999999);

    // 첫 번째 active job: totalProductionTimeMin=10 → elapsed=bigNow-0=999999 >= 600 → 완료
    ProductionJob job1 = makeJob(orderId1, sampleId1, 5, 7, 10, 0);
    // queue에 있는 두 번째 job: totalProductionTimeMin=20 → startTimeUnix=0이면 elapsed=bigNow-0=999999 >= 1200 → 완료
    ProductionJob job2 = makeJob(orderId2, sampleId2, 3, 4, 20, 0);

    // 1회차 getState: job1 active, job2 in queue
    ProductionState state1;
    state1.activeJob = job1;
    state1.queue.push_back(job2);

    // 2회차 getState: job2 active (startTimeUnix=0 → elapsed=bigNow-0=999999 >= 1200 → 완료)
    // (실제 저장소라면 startNextJob이 startTimeUnix=bigNow로 저장하지만,
    //  테스트에서는 연쇄 완료를 시뮬레이션하기 위해 startTimeUnix=0으로 설정)
    ProductionState state2;
    state2.activeJob = job2;  // startTimeUnix=0 유지 → 완료 조건 충족

    ON_CALL(mockTime, now())
        .WillByDefault(Return(bigNow));

    Sample sample1 = makeSample(sampleId1, 10);
    Sample sample2 = makeSample(sampleId2, 5);
    Order  order1  = makeOrder(orderId1, sampleId1, 20, OrderStatus::PRODUCING);
    Order  order2  = makeOrder(orderId2, sampleId2, 10, OrderStatus::PRODUCING);

    ON_CALL(mockSample, findById(sampleId1))
        .WillByDefault(Return(std::make_optional(sample1)));
    ON_CALL(mockSample, findById(sampleId2))
        .WillByDefault(Return(std::make_optional(sample2)));
    ON_CALL(mockOrder, findById(orderId1))
        .WillByDefault(Return(std::make_optional(order1)));
    ON_CALL(mockOrder, findById(orderId2))
        .WillByDefault(Return(std::make_optional(order2)));
    ON_CALL(mockProduction, setState(_))
        .WillByDefault(Return());

    // getState()는 순서대로 2회 호출:
    //   1회: job1 active + job2 queue
    //   2회: job2 active (queue 비어 있음, 완료 조건 충족) → 처리 후 return (3회차 없음)
    {
        InSequence seq;
        EXPECT_CALL(mockProduction, getState())
            .Times(2)
            .WillOnce(Return(state1))
            .WillOnce(Return(state2));
    }

    // 두 job 모두 완료 → 각각 update() 2회 호출
    EXPECT_CALL(mockSample, update(_))
        .Times(2)
        .WillRepeatedly(Return(true));
    EXPECT_CALL(mockOrder, update(_))
        .Times(2)
        .WillRepeatedly(Return(true));

    auto svc = makeService();
    svc.checkAndComplete();
}

// -----------------------------------------------------------------------
// TC-08: 재시작 내구성 (BR-16)
// startTimeUnix=1000(과거), totalProductionTimeMin=1, now()=2000
// elapsed=1000 > 60 → 완료 처리됨을 검증
// -----------------------------------------------------------------------
TEST_F(ProductionServiceTest, CheckAndComplete_RestartDurability_ProcessesPastJob)
{
    const std::string sampleId = "S-001";
    const std::string orderId  = "ORD-001";

    // 과거 시작, 1분짜리 job → 재시작 후에도 완료 처리
    ProductionJob job = makeJob(orderId, sampleId, 5, 7, 1, 1000LL);

    ProductionState state;
    state.activeJob = job;

    Sample sample = makeSample(sampleId, 10);
    Order  order  = makeOrder(orderId, sampleId, 20, OrderStatus::PRODUCING);

    ON_CALL(mockProduction, getState())
        .WillByDefault(Return(state));
    ON_CALL(mockTime, now())
        .WillByDefault(Return(static_cast<time_t>(2000)));
    ON_CALL(mockSample, findById(sampleId))
        .WillByDefault(Return(std::make_optional(sample)));
    ON_CALL(mockOrder, findById(orderId))
        .WillByDefault(Return(std::make_optional(order)));
    ON_CALL(mockProduction, setState(_))
        .WillByDefault(Return());

    // 완료 처리 → update() 각 1회 호출
    EXPECT_CALL(mockSample, update(_))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(mockOrder, update(_))
        .Times(1)
        .WillOnce(Return(true));

    auto svc = makeService();
    svc.checkAndComplete();
}
