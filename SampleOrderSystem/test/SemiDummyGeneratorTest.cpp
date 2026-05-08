// SemiDummyGeneratorTest.cpp
// SemiDummyGenerator 통합 테스트 (TDD 1단계 — 구현 없이 작성)
// 이 시점에서 구현이 없으므로 테스트는 FAIL 상태가 정상

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Tools/SemiDummyGenerator.h"
#include "../Model/Repository/JsonSampleRepository.h"
#include "../Model/Repository/JsonOrderRepository.h"
#include "../Model/Repository/JsonProductionRepository.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_set>
#include <cmath>
#include <algorithm>

namespace fs = std::filesystem;

// -----------------------------------------------------------------------
// Fixture
// -----------------------------------------------------------------------

class SemiDummyGeneratorTest : public ::testing::Test {
protected:
    std::string sampleFile;
    std::string orderFile;
    std::string productionFile;

    void SetUp() override {
        auto* info = ::testing::UnitTest::GetInstance()->current_test_info();
        std::string prefix = std::string("test_dummy_") + info->name() + "_";
        sampleFile     = (fs::temp_directory_path() / (prefix + "samples.json")).string();
        orderFile      = (fs::temp_directory_path() / (prefix + "orders.json")).string();
        productionFile = (fs::temp_directory_path() / (prefix + "production.json")).string();

        // 초기 빈 파일 생성
        std::ofstream(sampleFile)     << "{\"samples\":[]}";
        std::ofstream(orderFile)      << "{\"nextOrdNum\":1,\"orders\":[]}";
        std::ofstream(productionFile) << "{\"activeJob\":null,\"queue\":[]}";
    }

    void TearDown() override {
        fs::remove(sampleFile);
        fs::remove(orderFile);
        fs::remove(productionFile);
    }

    // Repository를 새로 로드하여 최신 파일 상태 반영
    std::vector<Sample> loadSamples() {
        JsonSampleRepository repo(sampleFile);
        return repo.findAll();
    }

    std::vector<Order> loadOrders() {
        JsonOrderRepository repo(orderFile);
        return repo.findAll();
    }

    ProductionState loadProductionState() {
        JsonProductionRepository repo(productionFile);
        return repo.getState();
    }
};

// -----------------------------------------------------------------------
// TC-01: run() 후 시료 수가 [5, 10] 범위 내에 있어야 함
// -----------------------------------------------------------------------
TEST_F(SemiDummyGeneratorTest, Run_GeneratesSamples_CountInRange)
{
    JsonSampleRepository     sampleRepo(sampleFile);
    JsonOrderRepository      orderRepo(orderFile);
    JsonProductionRepository productionRepo(productionFile);

    SemiDummyGenerator gen(sampleRepo, orderRepo, productionRepo);
    gen.run(false);

    auto samples = loadSamples();
    int count = static_cast<int>(samples.size());

    EXPECT_GE(count, 5)  << "시료 수는 최소 5개이어야 한다.";
    EXPECT_LE(count, 10) << "시료 수는 최대 10개이어야 한다.";
}

// -----------------------------------------------------------------------
// TC-02: BR-14 — 모든 시료의 yieldRate ∈ (0, 1.0]
// -----------------------------------------------------------------------
TEST_F(SemiDummyGeneratorTest, Run_Samples_YieldRateValid)
{
    JsonSampleRepository     sampleRepo(sampleFile);
    JsonOrderRepository      orderRepo(orderFile);
    JsonProductionRepository productionRepo(productionFile);

    SemiDummyGenerator gen(sampleRepo, orderRepo, productionRepo);
    gen.run(false);

    auto samples = loadSamples();
    ASSERT_FALSE(samples.empty()) << "시료가 1개 이상이어야 한다.";

    for (const auto& s : samples) {
        EXPECT_GT(s.yieldRate, 0.0)
            << "시료 [" << s.id << "]: yieldRate는 0 초과이어야 한다.";
        EXPECT_LE(s.yieldRate, 1.0)
            << "시료 [" << s.id << "]: yieldRate는 1.0 이하이어야 한다.";
    }
}

// -----------------------------------------------------------------------
// TC-03: 모든 시료의 avgProductionTime >= 1
// -----------------------------------------------------------------------
TEST_F(SemiDummyGeneratorTest, Run_Samples_AvgProductionTimeValid)
{
    JsonSampleRepository     sampleRepo(sampleFile);
    JsonOrderRepository      orderRepo(orderFile);
    JsonProductionRepository productionRepo(productionFile);

    SemiDummyGenerator gen(sampleRepo, orderRepo, productionRepo);
    gen.run(false);

    auto samples = loadSamples();
    ASSERT_FALSE(samples.empty()) << "시료가 1개 이상이어야 한다.";

    for (const auto& s : samples) {
        EXPECT_GE(s.avgProductionTime, 1)
            << "시료 [" << s.id << "]: avgProductionTime은 1 이상이어야 한다.";
    }
}

// -----------------------------------------------------------------------
// TC-04: run() 후 주문 수가 [10, 20] 범위 내에 있어야 함
// -----------------------------------------------------------------------
TEST_F(SemiDummyGeneratorTest, Run_GeneratesOrders_CountInRange)
{
    JsonSampleRepository     sampleRepo(sampleFile);
    JsonOrderRepository      orderRepo(orderFile);
    JsonProductionRepository productionRepo(productionFile);

    SemiDummyGenerator gen(sampleRepo, orderRepo, productionRepo);
    gen.run(false);

    auto orders = loadOrders();
    int count = static_cast<int>(orders.size());

    EXPECT_GE(count, 10) << "주문 수는 최소 10개이어야 한다.";
    EXPECT_LE(count, 20) << "주문 수는 최대 20개이어야 한다.";
}

// -----------------------------------------------------------------------
// TC-05: BR-01 — 모든 주문의 sampleId가 생성된 시료 목록에 존재해야 함
// -----------------------------------------------------------------------
TEST_F(SemiDummyGeneratorTest, Run_Orders_SampleIdExists)
{
    JsonSampleRepository     sampleRepo(sampleFile);
    JsonOrderRepository      orderRepo(orderFile);
    JsonProductionRepository productionRepo(productionFile);

    SemiDummyGenerator gen(sampleRepo, orderRepo, productionRepo);
    gen.run(false);

    auto samples = loadSamples();
    auto orders  = loadOrders();

    // 생성된 시료 ID 집합
    std::unordered_set<std::string> sampleIds;
    for (const auto& s : samples)
        sampleIds.insert(s.id);

    ASSERT_FALSE(orders.empty())  << "주문이 1개 이상이어야 한다.";
    ASSERT_FALSE(sampleIds.empty()) << "시료가 1개 이상이어야 한다.";

    for (const auto& o : orders) {
        EXPECT_NE(sampleIds.find(o.sampleId), sampleIds.end())
            << "주문 [" << o.id << "]: sampleId [" << o.sampleId
            << "]가 생성된 시료 목록에 없다. (BR-01 위반)";
    }
}

// -----------------------------------------------------------------------
// TC-06: BR-04, BR-13 — PRODUCING 주문이 activeJob 또는 queue에 연동되어야 하며
//        activeJob은 최대 1개여야 함
// -----------------------------------------------------------------------
TEST_F(SemiDummyGeneratorTest, Run_ProducingOrders_LinkedToProduction)
{
    JsonSampleRepository     sampleRepo(sampleFile);
    JsonOrderRepository      orderRepo(orderFile);
    JsonProductionRepository productionRepo(productionFile);

    SemiDummyGenerator gen(sampleRepo, orderRepo, productionRepo);
    gen.run(false);

    auto orders = loadOrders();
    auto state  = loadProductionState();

    // PRODUCING 상태 주문 수집
    std::vector<Order> producingOrders;
    for (const auto& o : orders) {
        if (o.status == OrderStatus::PRODUCING)
            producingOrders.push_back(o);
    }

    // activeJob은 최대 1개 (BR-13)
    int activeJobCount = state.activeJob.has_value() ? 1 : 0;
    EXPECT_LE(activeJobCount, 1) << "activeJob은 최대 1개이어야 한다. (BR-13)";

    if (producingOrders.empty()) {
        // PRODUCING 주문이 없으면 activeJob도 없어야 함
        EXPECT_FALSE(state.activeJob.has_value())
            << "PRODUCING 주문이 없으면 activeJob도 없어야 한다.";
        EXPECT_TRUE(state.queue.empty())
            << "PRODUCING 주문이 없으면 queue도 비어야 한다.";
        return;
    }

    // 모든 PRODUCING 주문의 orderId가 activeJob 또는 queue에 존재해야 함
    std::unordered_set<std::string> productionOrderIds;
    if (state.activeJob.has_value())
        productionOrderIds.insert(state.activeJob->orderId);
    for (const auto& job : state.queue)
        productionOrderIds.insert(job.orderId);

    for (const auto& o : producingOrders) {
        EXPECT_NE(productionOrderIds.find(o.id), productionOrderIds.end())
            << "PRODUCING 주문 [" << o.id << "]이 production에 연동되지 않았다. (BR-04)";
    }

    // production의 모든 job도 PRODUCING 주문에 대응해야 함
    std::unordered_set<std::string> producingOrderIds;
    for (const auto& o : producingOrders)
        producingOrderIds.insert(o.id);

    if (state.activeJob.has_value()) {
        EXPECT_NE(producingOrderIds.find(state.activeJob->orderId), producingOrderIds.end())
            << "activeJob의 orderId [" << state.activeJob->orderId
            << "]에 대응하는 PRODUCING 주문이 없다.";
    }
    for (const auto& job : state.queue) {
        EXPECT_NE(producingOrderIds.find(job.orderId), producingOrderIds.end())
            << "queue의 orderId [" << job.orderId
            << "]에 대응하는 PRODUCING 주문이 없다.";
    }
}

// -----------------------------------------------------------------------
// TC-07: BR-06, BR-07 — PRODUCING 주문의 actualProductionQty 및
//        totalProductionTimeMin 계산 검증
//        actualProductionQty = ceil(shortage / (yieldRate * 0.9))
//        totalProductionTimeMin = avgProductionTime * actualProductionQty
// -----------------------------------------------------------------------
TEST_F(SemiDummyGeneratorTest, Run_ProducingOrders_ProductionQtyCalculation)
{
    JsonSampleRepository     sampleRepo(sampleFile);
    JsonOrderRepository      orderRepo(orderFile);
    JsonProductionRepository productionRepo(productionFile);

    SemiDummyGenerator gen(sampleRepo, orderRepo, productionRepo);
    gen.run(false);

    auto samples = loadSamples();
    auto orders  = loadOrders();
    auto state   = loadProductionState();

    // 시료 ID → 시료 매핑
    std::unordered_map<std::string, Sample> sampleMap;
    for (const auto& s : samples)
        sampleMap[s.id] = s;

    // production job 수집 (activeJob + queue)
    std::vector<ProductionJob> allJobs;
    if (state.activeJob.has_value())
        allJobs.push_back(*state.activeJob);
    for (const auto& job : state.queue)
        allJobs.push_back(job);

    if (allJobs.empty()) {
        // PRODUCING 주문이 없는 경우 → 검증 불필요
        return;
    }

    // 주문 ID → 주문 매핑
    std::unordered_map<std::string, Order> orderMap;
    for (const auto& o : orders)
        orderMap[o.id] = o;

    for (const auto& job : allJobs) {
        ASSERT_NE(sampleMap.find(job.sampleId), sampleMap.end())
            << "job.sampleId [" << job.sampleId << "]에 대응하는 시료가 없다.";
        ASSERT_NE(orderMap.find(job.orderId), orderMap.end())
            << "job.orderId [" << job.orderId << "]에 대응하는 주문이 없다.";

        const Sample& sample = sampleMap[job.sampleId];

        // BR-06: actualProductionQty = ceil(shortage / (yieldRate * 0.9))
        int expectedActualQty = static_cast<int>(
            std::ceil(static_cast<double>(job.shortage) / (sample.yieldRate * 0.9))
        );
        EXPECT_EQ(job.actualProductionQty, expectedActualQty)
            << "job [orderId=" << job.orderId << "]: actualProductionQty 계산 오류. "
            << "기대=" << expectedActualQty
            << ", 실제=" << job.actualProductionQty
            << " (BR-06)";

        // BR-07: totalProductionTimeMin = avgProductionTime * actualProductionQty
        int expectedTotalTime = sample.avgProductionTime * job.actualProductionQty;
        EXPECT_EQ(job.totalProductionTimeMin, expectedTotalTime)
            << "job [orderId=" << job.orderId << "]: totalProductionTimeMin 계산 오류. "
            << "기대=" << expectedTotalTime
            << ", 실제=" << job.totalProductionTimeMin
            << " (BR-07)";
    }
}

// -----------------------------------------------------------------------
// TC-08: BR-08 — queue는 FIFO (주문 생성 순서와 동일해야 함)
//        PRODUCING 주문 중 activeJob 이후 항목들의 orderId 순서 검증
// -----------------------------------------------------------------------
TEST_F(SemiDummyGeneratorTest, Run_QueueFifoOrder)
{
    JsonSampleRepository     sampleRepo(sampleFile);
    JsonOrderRepository      orderRepo(orderFile);
    JsonProductionRepository productionRepo(productionFile);

    SemiDummyGenerator gen(sampleRepo, orderRepo, productionRepo);
    gen.run(false);

    auto orders = loadOrders();
    auto state  = loadProductionState();

    if (state.queue.size() < 2) {
        // queue가 2개 미만이면 순서 검증 불필요
        SUCCEED() << "queue가 2개 미만이므로 FIFO 순서 검증 생략";
        return;
    }

    // PRODUCING 주문을 ID 기준으로 생성 순서 파악 (ORD-NNN 형식 → 숫자 오름차순)
    std::vector<Order> producingOrders;
    for (const auto& o : orders) {
        if (o.status == OrderStatus::PRODUCING)
            producingOrders.push_back(o);
    }

    // orderId로 번호 추출 (ORD-001 → 1)
    auto extractNum = [](const std::string& id) -> int {
        auto pos = id.rfind('-');
        if (pos == std::string::npos) return 0;
        try { return std::stoi(id.substr(pos + 1)); }
        catch (...) { return 0; }
    };

    // queue의 orderId를 번호 순으로 정렬
    std::vector<int> queueNums;
    for (const auto& job : state.queue)
        queueNums.push_back(extractNum(job.orderId));

    // FIFO: queue의 번호가 단조 증가해야 함 (BR-08)
    for (size_t i = 1; i < queueNums.size(); ++i) {
        EXPECT_LT(queueNums[i - 1], queueNums[i])
            << "queue[" << (i - 1) << "].orderId=" << state.queue[i - 1].orderId
            << " 가 queue[" << i << "].orderId=" << state.queue[i].orderId
            << " 보다 나중에 생성되었다. FIFO 순서 위반 (BR-08)";
    }
}

// -----------------------------------------------------------------------
// TC-09: run(true) — append 모드: 기존 데이터에 추가되어 시료 수 ≥ 이전 시료 수
// -----------------------------------------------------------------------
TEST_F(SemiDummyGeneratorTest, Run_Append_AddsToExisting)
{
    JsonSampleRepository     sampleRepo1(sampleFile);
    JsonOrderRepository      orderRepo1(orderFile);
    JsonProductionRepository productionRepo1(productionFile);

    SemiDummyGenerator gen1(sampleRepo1, orderRepo1, productionRepo1);
    gen1.run(false);

    int firstRunSampleCount = static_cast<int>(loadSamples().size());
    int firstRunOrderCount  = static_cast<int>(loadOrders().size());

    // 두 번째 run: append 모드
    JsonSampleRepository     sampleRepo2(sampleFile);
    JsonOrderRepository      orderRepo2(orderFile);
    JsonProductionRepository productionRepo2(productionFile);

    SemiDummyGenerator gen2(sampleRepo2, orderRepo2, productionRepo2);
    gen2.run(true);

    int secondRunSampleCount = static_cast<int>(loadSamples().size());
    int secondRunOrderCount  = static_cast<int>(loadOrders().size());

    EXPECT_GE(secondRunSampleCount, firstRunSampleCount)
        << "append 모드에서 시료 수는 줄어들면 안 된다.";
    EXPECT_GE(secondRunOrderCount, firstRunOrderCount)
        << "append 모드에서 주문 수는 줄어들면 안 된다.";
}

// -----------------------------------------------------------------------
// TC-10: run(false) 두 번 — 두 번째 결과만 남아야 함 (덮어쓰기)
//        주문 개수로 확인 (두 번째 run의 주문 수 ∈ [10, 20])
// -----------------------------------------------------------------------
TEST_F(SemiDummyGeneratorTest, Run_NotAppend_OverwritesExisting)
{
    // 첫 번째 run
    {
        JsonSampleRepository     sampleRepo(sampleFile);
        JsonOrderRepository      orderRepo(orderFile);
        JsonProductionRepository productionRepo(productionFile);

        SemiDummyGenerator gen(sampleRepo, orderRepo, productionRepo);
        gen.run(false);
    }

    int firstRunOrderCount = static_cast<int>(loadOrders().size());
    EXPECT_GE(firstRunOrderCount, 10) << "첫 번째 run 후 주문 수는 10개 이상이어야 한다.";

    // 두 번째 run (덮어쓰기)
    {
        // 새 Repository 인스턴스로 새로 로드
        JsonSampleRepository     sampleRepo(sampleFile);
        JsonOrderRepository      orderRepo(orderFile);
        JsonProductionRepository productionRepo(productionFile);

        SemiDummyGenerator gen(sampleRepo, orderRepo, productionRepo);
        gen.run(false);
    }

    int secondRunOrderCount = static_cast<int>(loadOrders().size());

    // 두 번째 run(false)만의 결과이므로 [10, 20] 범위 내에 있어야 함
    EXPECT_GE(secondRunOrderCount, 10)
        << "두 번째 run(false) 후 주문 수는 10개 이상이어야 한다.";
    EXPECT_LE(secondRunOrderCount, 20)
        << "두 번째 run(false) 후 주문 수는 20개 이하이어야 한다. "
        << "(덮어쓰기가 아니면 누적되어 20을 초과할 수 있다.)";
}
