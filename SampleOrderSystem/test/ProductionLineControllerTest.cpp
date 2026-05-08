// ProductionLineControllerTest.cpp
// ProductionLineController 단위 테스트 (TDD — 구현 없이 먼저 작성)
// 이 시점에서 구현이 없으므로 테스트는 FAIL 상태가 정상

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Model/Repository/IProductionRepository.h"
#include "../Model/Repository/ISampleRepository.h"
#include "../Model/Service/ITimeProvider.h"
#include "../Model/Service/MockTimeProvider.h"
#include "../View/IProductionLineView.h"
#include "../Controller/ProductionLineController.h"
#include "../Model/Service/IProductionService.h"

using ::testing::Return;
using ::testing::_;
using ::testing::Eq;
using ::testing::InSequence;

// -----------------------------------------------------------------------
// Mock 클래스
// -----------------------------------------------------------------------

class MockProductionService : public IProductionService {
public:
    MOCK_METHOD(void, checkAndComplete, (), (override));
};

class MockProductionRepository : public IProductionRepository {
public:
    MOCK_METHOD(ProductionState, getState, (), (const, override));
    MOCK_METHOD(void, setState, (const ProductionState&), (override));
    MOCK_METHOD(void, enqueue, (const ProductionJob&), (override));
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

class MockProductionLineView : public IProductionLineView {
public:
    MOCK_METHOD(void, showSubMenu, (bool), (override));
    MOCK_METHOD(int, getSubMenuChoice, (), (override));
    MOCK_METHOD(void, showActiveJob,
        (const ProductionJob&, const std::string&, int, const std::string&),
        (override));
    MOCK_METHOD(void, showNoActiveJob, (), (override));
    MOCK_METHOD(void, showQueue,
        (const std::vector<ProductionJob>&, const std::vector<std::string>&),
        (override));
    MOCK_METHOD(int, promptAdvanceMinutes, (), (override));
    MOCK_METHOD(void, showError, (const std::string&), (override));
};

// -----------------------------------------------------------------------
// 헬퍼
// -----------------------------------------------------------------------

static ProductionJob makeJob(
    const std::string& orderId,
    const std::string& sampleId,
    int64_t startTimeUnix,
    int totalMin)
{
    ProductionJob job;
    job.orderId               = orderId;
    job.sampleId              = sampleId;
    job.shortage              = 10;
    job.actualProductionQty   = 10;
    job.totalProductionTimeMin = totalMin;
    job.startTimeUnix         = startTimeUnix;
    return job;
}

static Sample makeSample(const std::string& id, const std::string& name)
{
    Sample s;
    s.id                = id;
    s.name              = name;
    s.avgProductionTime = 10;
    s.yieldRate         = 0.9;
    s.currentStock      = 0;
    return s;
}

// -----------------------------------------------------------------------
// Fixture
// -----------------------------------------------------------------------

class ProductionLineControllerTest : public ::testing::Test {
protected:
    MockProductionService    mockService;
    MockProductionRepository mockProductionRepo;
    MockSampleRepository     mockSampleRepo;
    MockProductionLineView   mockView;

    // normalMode=false → isMockMode=false
    ProductionLineController makeController(bool isMockMode = false) {
        return ProductionLineController(
            mockService,
            mockProductionRepo,
            mockSampleRepo,
            timeProvider,
            mockView,
            isMockMode);
    }

    // 실제 MockTimeProvider — advance()를 호출해 간접 검증에 사용
    MockTimeProvider timeProvider{ static_cast<time_t>(1600) };
};

// -----------------------------------------------------------------------
// TC-01: showStatus() — activeJob 없음 → showNoActiveJob() 1회, showActiveJob() 0회
// -----------------------------------------------------------------------
TEST_F(ProductionLineControllerTest, ShowStatus_NoActiveJob_TriggersCheckAndCompleteAndShowsNoActiveJob)
{
    ProductionState state;
    state.activeJob = std::nullopt;

    EXPECT_CALL(mockService, checkAndComplete())
        .Times(1);

    EXPECT_CALL(mockProductionRepo, getState())
        .Times(1)
        .WillOnce(Return(state));

    EXPECT_CALL(mockView, showNoActiveJob())
        .Times(1);

    EXPECT_CALL(mockView, showActiveJob(_, _, _, _))
        .Times(0);

    auto ctrl = makeController();
    ctrl.showStatus();
}

// -----------------------------------------------------------------------
// TC-02: showStatus() — activeJob 있음 → showActiveJob() 1회, showNoActiveJob() 0회
// startTimeUnix=1000, now()=1600 → elapsedMinutes = (1600-1000)/60 = 10
// sampleId="S-001" → name "AlGaN"
// -----------------------------------------------------------------------
TEST_F(ProductionLineControllerTest, ShowStatus_WithActiveJob_ShowsActiveJobInfo)
{
    const int64_t startTime = 1000;
    const int     totalMin  = 10;

    ProductionJob job = makeJob("ORD-001", "S-001", startTime, totalMin);

    ProductionState state;
    state.activeJob = job;

    Sample sample = makeSample("S-001", "AlGaN");

    // timeProvider.now() == 1600 (생성자에서 설정)
    // elapsedMinutes = (1600 - 1000) / 60 = 10

    EXPECT_CALL(mockService, checkAndComplete())
        .Times(1);

    EXPECT_CALL(mockProductionRepo, getState())
        .Times(1)
        .WillOnce(Return(state));

    EXPECT_CALL(mockSampleRepo, findById(Eq(std::string("S-001"))))
        .Times(1)
        .WillOnce(Return(std::make_optional(sample)));

    EXPECT_CALL(mockView, showActiveJob(_, Eq(std::string("AlGaN")), Eq(10), _))
        .Times(1);

    EXPECT_CALL(mockView, showNoActiveJob())
        .Times(0);

    auto ctrl = makeController();
    ctrl.showStatus();
}

// -----------------------------------------------------------------------
// TC-04: showQueue() — queue 비어 있음 → showQueue(empty, empty) 1회 호출
// -----------------------------------------------------------------------
TEST_F(ProductionLineControllerTest, ShowQueue_EmptyQueue_CallsShowQueue)
{
    ProductionState state;
    // activeJob 없음, queue 비어 있음

    EXPECT_CALL(mockProductionRepo, getState())
        .Times(1)
        .WillOnce(Return(state));

    EXPECT_CALL(mockView, showQueue(
        Eq(std::vector<ProductionJob>{}),
        Eq(std::vector<std::string>{})))
        .Times(1);

    auto ctrl = makeController();
    ctrl.showQueue();
}

// -----------------------------------------------------------------------
// TC-05: showQueue() — queue에 job 2개 → sampleNames 순서대로 view에 전달
// job1.sampleId="S-001" → "AlGaN", job2.sampleId="S-002" → "GaN"
// -----------------------------------------------------------------------
TEST_F(ProductionLineControllerTest, ShowQueue_TwoJobs_PassesSampleNamesToView)
{
    ProductionJob job1 = makeJob("ORD-001", "S-001", 1000, 10);
    ProductionJob job2 = makeJob("ORD-002", "S-002", 2000, 20);

    ProductionState state;
    state.queue.push_back(job1);
    state.queue.push_back(job2);

    Sample sample1 = makeSample("S-001", "AlGaN");
    Sample sample2 = makeSample("S-002", "GaN");

    EXPECT_CALL(mockProductionRepo, getState())
        .Times(1)
        .WillOnce(Return(state));

    EXPECT_CALL(mockSampleRepo, findById(Eq(std::string("S-001"))))
        .WillOnce(Return(std::make_optional(sample1)));

    EXPECT_CALL(mockSampleRepo, findById(Eq(std::string("S-002"))))
        .WillOnce(Return(std::make_optional(sample2)));

    std::vector<std::string> expectedNames = { "AlGaN", "GaN" };
    std::vector<ProductionJob> expectedJobs = { job1, job2 };

    EXPECT_CALL(mockView, showQueue(Eq(expectedJobs), Eq(expectedNames)))
        .Times(1);

    auto ctrl = makeController();
    ctrl.showQueue();
}

// -----------------------------------------------------------------------
// TC-06: advanceTime() — mock 모드 전용
// advanceTime(30) 호출 → MockTimeProvider::advance(30) 경유 now()가 1600 + 30*60 = 3400 으로 증가
// checkAndComplete() 1회 호출
// -----------------------------------------------------------------------
TEST_F(ProductionLineControllerTest, AdvanceTime_MockMode_AdvancesTimeAndChecks)
{
    const int advanceMin = 30;
    const time_t expectedNow = 1600 + static_cast<time_t>(advanceMin) * 60; // 3400

    EXPECT_CALL(mockService, checkAndComplete())
        .Times(1);

    // isMockMode=true
    auto ctrl = makeController(true);
    ctrl.advanceTime(advanceMin);

    // MockTimeProvider::advance(30) 가 실제로 호출되었으면 now()가 3400이어야 함
    EXPECT_EQ(timeProvider.now(), expectedNow);
}
