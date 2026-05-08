// ProductionRepositoryTest.cpp
// JsonProductionRepository 단위 테스트
// 구현 코드 없이 작성된 TDD 테스트 파일 (테스트는 구현 전이므로 FAIL 상태가 정상)

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Model/Repository/JsonProductionRepository.h"

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// -----------------------------------------------------------------------
// 헬퍼
// -----------------------------------------------------------------------

/// 테스트용 ProductionJob 생성 헬퍼
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

// -----------------------------------------------------------------------
// Fixture
// -----------------------------------------------------------------------

class ProductionRepositoryTest : public ::testing::Test
{
protected:
    std::string tempFilePath;

    void SetUp() override
    {
        const ::testing::TestInfo* info =
            ::testing::UnitTest::GetInstance()->current_test_info();
        tempFilePath = (fs::temp_directory_path() /
            (std::string("test_prod_") + info->test_suite_name() + "_" + info->name() + ".json")
        ).string();
    }

    void TearDown() override
    {
        if (fs::exists(tempFilePath))
        {
            fs::remove(tempFilePath);
        }
    }
};

// -----------------------------------------------------------------------
// TC-01: 빈 파일(또는 파일 없음) → activeJob = nullopt, queue 비어있음
// -----------------------------------------------------------------------
TEST_F(ProductionRepositoryTest, GetState_EmptyFile_ActiveJobIsNullopt)
{
    JsonProductionRepository repo(tempFilePath);
    ProductionState state = repo.getState();

    EXPECT_FALSE(state.activeJob.has_value());
    EXPECT_TRUE(state.queue.empty());
}

// -----------------------------------------------------------------------
// TC-02: enqueue(job) 후 getState().queue.size() == 1
// -----------------------------------------------------------------------
TEST_F(ProductionRepositoryTest, Enqueue_OneJob_QueueSizeIsOne)
{
    JsonProductionRepository repo(tempFilePath);
    repo.enqueue(makeJob("ORD-001"));

    ProductionState state = repo.getState();
    EXPECT_EQ(state.queue.size(), 1u);
}

// -----------------------------------------------------------------------
// TC-03: enqueue 두 번 → queue[0].orderId="ORD-001", queue[1].orderId="ORD-002" (FIFO, BR-08)
// -----------------------------------------------------------------------
TEST_F(ProductionRepositoryTest, Enqueue_TwoJobs_FifoOrder)
{
    JsonProductionRepository repo(tempFilePath);
    repo.enqueue(makeJob("ORD-001"));
    repo.enqueue(makeJob("ORD-002"));

    ProductionState state = repo.getState();
    ASSERT_EQ(state.queue.size(), 2u);
    EXPECT_EQ(state.queue[0].orderId, "ORD-001");
    EXPECT_EQ(state.queue[1].orderId, "ORD-002");
}

// -----------------------------------------------------------------------
// TC-04: activeJob 있는 ProductionState 저장 후 getState()로 모든 필드 검증
// -----------------------------------------------------------------------
TEST_F(ProductionRepositoryTest, SetState_WithActiveJob_GetStateReturnsIt)
{
    ProductionJob job = makeJob("ORD-010", "S-042", 3, 8, 40, 1746700000LL);

    ProductionState stateToSave;
    stateToSave.activeJob = job;

    JsonProductionRepository repo(tempFilePath);
    repo.setState(stateToSave);

    ProductionState loaded = repo.getState();
    ASSERT_TRUE(loaded.activeJob.has_value());
    const ProductionJob& a = loaded.activeJob.value();
    EXPECT_EQ(a.orderId,                "ORD-010");
    EXPECT_EQ(a.sampleId,               "S-042");
    EXPECT_EQ(a.shortage,               3);
    EXPECT_EQ(a.actualProductionQty,    8);
    EXPECT_EQ(a.totalProductionTimeMin, 40);
    EXPECT_EQ(a.startTimeUnix,          1746700000LL);
}

// -----------------------------------------------------------------------
// TC-05: queue 2개 저장 후 재조회 시 동일 데이터
// -----------------------------------------------------------------------
TEST_F(ProductionRepositoryTest, SetState_WithQueue_GetStateReturnsQueue)
{
    ProductionState stateToSave;
    stateToSave.queue.push_back(makeJob("ORD-001", "S-001", 5, 7, 35, 0));
    stateToSave.queue.push_back(makeJob("ORD-002", "S-002", 2, 4, 20, 100));

    JsonProductionRepository repo(tempFilePath);
    repo.setState(stateToSave);

    ProductionState loaded = repo.getState();
    ASSERT_EQ(loaded.queue.size(), 2u);
    EXPECT_EQ(loaded.queue[0].orderId,                "ORD-001");
    EXPECT_EQ(loaded.queue[0].sampleId,               "S-001");
    EXPECT_EQ(loaded.queue[0].shortage,               5);
    EXPECT_EQ(loaded.queue[0].actualProductionQty,    7);
    EXPECT_EQ(loaded.queue[0].totalProductionTimeMin, 35);
    EXPECT_EQ(loaded.queue[1].orderId,                "ORD-002");
    EXPECT_EQ(loaded.queue[1].sampleId,               "S-002");
    EXPECT_EQ(loaded.queue[1].shortage,               2);
    EXPECT_EQ(loaded.queue[1].actualProductionQty,    4);
    EXPECT_EQ(loaded.queue[1].totalProductionTimeMin, 20);
}

// -----------------------------------------------------------------------
// TC-06: setState 후 Repository 재생성 → activeJob 유지 (재시작 내구성, BR-16)
// -----------------------------------------------------------------------
TEST_F(ProductionRepositoryTest, Persistence_AfterReload_ActiveJobPreserved)
{
    ProductionJob job = makeJob("ORD-007", "S-007", 1, 2, 10, 999LL);

    // 첫 번째 인스턴스: activeJob 저장 후 소멸
    {
        JsonProductionRepository repo(tempFilePath);
        ProductionState s;
        s.activeJob = job;
        repo.setState(s);
    }

    // 두 번째 인스턴스: 같은 파일로 재생성 → activeJob 유지 확인
    {
        JsonProductionRepository repo(tempFilePath);
        ProductionState loaded = repo.getState();
        ASSERT_TRUE(loaded.activeJob.has_value());
        const ProductionJob& a = loaded.activeJob.value();
        EXPECT_EQ(a.orderId,                "ORD-007");
        EXPECT_EQ(a.sampleId,               "S-007");
        EXPECT_EQ(a.shortage,               1);
        EXPECT_EQ(a.actualProductionQty,    2);
        EXPECT_EQ(a.totalProductionTimeMin, 10);
        EXPECT_EQ(a.startTimeUnix,          999LL);
    }
}

// -----------------------------------------------------------------------
// TC-07: 재생성 후 queue 전체 유지 (재시작 내구성, BR-16)
// -----------------------------------------------------------------------
TEST_F(ProductionRepositoryTest, Persistence_AfterReload_QueuePreserved)
{
    // 첫 번째 인스턴스: queue 2개 저장 후 소멸
    {
        JsonProductionRepository repo(tempFilePath);
        repo.enqueue(makeJob("ORD-001"));
        repo.enqueue(makeJob("ORD-002"));
    }

    // 두 번째 인스턴스: 같은 파일로 재생성 → queue 전체 유지
    {
        JsonProductionRepository repo(tempFilePath);
        ProductionState loaded = repo.getState();
        ASSERT_EQ(loaded.queue.size(), 2u);
        EXPECT_EQ(loaded.queue[0].orderId, "ORD-001");
        EXPECT_EQ(loaded.queue[1].orderId, "ORD-002");
    }
}

// -----------------------------------------------------------------------
// TC-08: activeJob=nullopt → 파일 저장 후 재로드 시 nullopt
// -----------------------------------------------------------------------
TEST_F(ProductionRepositoryTest, SetState_NullActiveJob_SavedAndLoaded)
{
    // 먼저 activeJob을 설정했다가
    {
        JsonProductionRepository repo(tempFilePath);
        ProductionState s;
        s.activeJob = makeJob("ORD-001");
        repo.setState(s);
    }

    // 그 다음 activeJob=nullopt로 덮어씀
    {
        JsonProductionRepository repo(tempFilePath);
        ProductionState s;
        s.activeJob = std::nullopt;
        repo.setState(s);
    }

    // 재로드 시 nullopt 확인
    {
        JsonProductionRepository repo(tempFilePath);
        ProductionState loaded = repo.getState();
        EXPECT_FALSE(loaded.activeJob.has_value());
    }
}

// -----------------------------------------------------------------------
// TC-09: setState(queue 1개) 후 enqueue → queue.size()==2
// -----------------------------------------------------------------------
TEST_F(ProductionRepositoryTest, Enqueue_AfterSetState_AppendsToQueue)
{
    JsonProductionRepository repo(tempFilePath);

    // queue에 1개 있는 상태로 setState
    ProductionState s;
    s.queue.push_back(makeJob("ORD-001"));
    repo.setState(s);

    // enqueue 1개 추가
    repo.enqueue(makeJob("ORD-002"));

    ProductionState loaded = repo.getState();
    EXPECT_EQ(loaded.queue.size(), 2u);
    EXPECT_EQ(loaded.queue[0].orderId, "ORD-001");
    EXPECT_EQ(loaded.queue[1].orderId, "ORD-002");
}
