// OrderRepositoryTest.cpp
// JsonOrderRepository 단위 테스트
// 구현 코드 없이 작성된 TDD 테스트 파일 (테스트는 구현 전이므로 FAIL 상태가 정상)

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Model/Repository/JsonOrderRepository.h"

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// -----------------------------------------------------------------------
// 헬퍼
// -----------------------------------------------------------------------

/// 테스트용 Order 객체 생성 헬퍼 (id는 create()가 할당하므로 빈 문자열)
static Order makeOrder(
    const std::string& sampleId     = "S-001",
    const std::string& customerName = "테스트고객",
    int                quantity     = 10,
    OrderStatus        status       = OrderStatus::RESERVED,
    const std::string& createdAt    = "2026-05-08T10:00:00")
{
    Order o;
    o.id           = "";
    o.sampleId     = sampleId;
    o.customerName = customerName;
    o.quantity     = quantity;
    o.status       = status;
    o.createdAt    = createdAt;
    return o;
}

// -----------------------------------------------------------------------
// Fixture
// -----------------------------------------------------------------------

class OrderRepositoryTest : public ::testing::Test
{
protected:
    std::string tempFilePath;

    void SetUp() override
    {
        const ::testing::TestInfo* info =
            ::testing::UnitTest::GetInstance()->current_test_info();
        tempFilePath = (fs::temp_directory_path() /
            (std::string("test_orders_") + info->test_suite_name() + "_" + info->name() + ".json")
        ).string();
    }

    void TearDown() override
    {
        if (fs::exists(tempFilePath))
        {
            fs::remove(tempFilePath);
        }
    }

    JsonOrderRepository makeRepo() const
    {
        return JsonOrderRepository(tempFilePath);
    }
};

// -----------------------------------------------------------------------
// TC-01: create() 첫 번째 호출 → "ORD-001" 반환
// -----------------------------------------------------------------------
TEST_F(OrderRepositoryTest, Create_FirstOrder_ReturnsOrd001)
{
    auto repo = makeRepo();
    std::string id = repo.create(makeOrder());
    EXPECT_EQ(id, "ORD-001");
}

// -----------------------------------------------------------------------
// TC-02: create() 두 번 호출 → "ORD-001", "ORD-002" 순차 생성
// -----------------------------------------------------------------------
TEST_F(OrderRepositoryTest, Create_SecondOrder_ReturnsOrd002)
{
    auto repo = makeRepo();
    std::string id1 = repo.create(makeOrder());
    std::string id2 = repo.create(makeOrder("S-002", "고객B"));
    EXPECT_EQ(id1, "ORD-001");
    EXPECT_EQ(id2, "ORD-002");
}

// -----------------------------------------------------------------------
// TC-03: 10개 create 후 마지막 ID가 3자리 zero-padding 확인
//        nextOrdNum=9  → "ORD-009"
//        nextOrdNum=10 → "ORD-010"
// -----------------------------------------------------------------------
TEST_F(OrderRepositoryTest, Create_OrderIdZeroPadded_ThreeDigits)
{
    auto repo = makeRepo();

    std::string id9th, id10th;
    for (int i = 0; i < 10; ++i)
    {
        std::string id = repo.create(makeOrder());
        if (i == 8)  id9th  = id;
        if (i == 9)  id10th = id;
    }

    EXPECT_EQ(id9th,  "ORD-009");
    EXPECT_EQ(id10th, "ORD-010");
}

// -----------------------------------------------------------------------
// TC-04: Repository 재생성 후 nextOrdNum 이어서 증가 (재시작 내구성)
// -----------------------------------------------------------------------
TEST_F(OrderRepositoryTest, Persistence_NextOrdNum_ContinuesAfterReload)
{
    // 첫 번째 인스턴스: 2개 생성 후 소멸
    {
        JsonOrderRepository repo(tempFilePath);
        repo.create(makeOrder());
        repo.create(makeOrder());
    }

    // 두 번째 인스턴스: 같은 파일로 재생성 → nextOrdNum=3에서 이어짐
    {
        JsonOrderRepository repo(tempFilePath);
        std::string id = repo.create(makeOrder());
        EXPECT_EQ(id, "ORD-003");
    }
}

// -----------------------------------------------------------------------
// TC-05: create 후 findById()로 모든 필드 검증
// -----------------------------------------------------------------------
TEST_F(OrderRepositoryTest, FindById_ExistingId_ReturnsOrder)
{
    auto repo = makeRepo();
    Order o = makeOrder("S-042", "김연구원", 25, OrderStatus::RESERVED, "2026-05-08T10:00:00");
    std::string id = repo.create(o);

    auto result = repo.findById(id);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id,           "ORD-001");
    EXPECT_EQ(result->sampleId,     "S-042");
    EXPECT_EQ(result->customerName, "김연구원");
    EXPECT_EQ(result->quantity,     25);
    EXPECT_EQ(result->status,       OrderStatus::RESERVED);
    EXPECT_EQ(result->createdAt,    "2026-05-08T10:00:00");
}

// -----------------------------------------------------------------------
// TC-06: findById() — 없는 ID → nullopt 반환
// -----------------------------------------------------------------------
TEST_F(OrderRepositoryTest, FindById_NonExistingId_ReturnsNullopt)
{
    auto repo = makeRepo();
    auto result = repo.findById("ORD-999");
    EXPECT_FALSE(result.has_value());
}

// -----------------------------------------------------------------------
// TC-07: findAll() — 3개 create 후 3개 반환
// -----------------------------------------------------------------------
TEST_F(OrderRepositoryTest, FindAll_ThreeOrders_ReturnsAll)
{
    auto repo = makeRepo();
    repo.create(makeOrder("S-001", "고객A"));
    repo.create(makeOrder("S-002", "고객B"));
    repo.create(makeOrder("S-003", "고객C"));

    auto all = repo.findAll();
    EXPECT_EQ(all.size(), 3u);
}

// -----------------------------------------------------------------------
// TC-08: findByStatus(RESERVED) — RESERVED 2개·CONFIRMED 1개 저장 후 2개만 반환
// -----------------------------------------------------------------------
TEST_F(OrderRepositoryTest, FindByStatus_Reserved_ReturnsOnlyReserved)
{
    auto repo = makeRepo();
    repo.create(makeOrder("S-001", "고객A", 10, OrderStatus::RESERVED));
    repo.create(makeOrder("S-002", "고객B", 20, OrderStatus::RESERVED));

    // 세 번째 주문은 create 후 update로 CONFIRMED 상태로 변경
    std::string id3 = repo.create(makeOrder("S-003", "고객C", 30, OrderStatus::RESERVED));
    auto o3 = repo.findById(id3);
    ASSERT_TRUE(o3.has_value());
    o3->status = OrderStatus::CONFIRMED;
    repo.update(*o3);

    auto reserved = repo.findByStatus(OrderStatus::RESERVED);
    EXPECT_EQ(reserved.size(), 2u);
    for (const auto& order : reserved)
    {
        EXPECT_EQ(order.status, OrderStatus::RESERVED);
    }
}

// -----------------------------------------------------------------------
// TC-09: findByStatus() — 매칭 결과 없음 → 빈 벡터
// -----------------------------------------------------------------------
TEST_F(OrderRepositoryTest, FindByStatus_NoMatch_ReturnsEmpty)
{
    auto repo = makeRepo();
    repo.create(makeOrder("S-001", "고객A", 10, OrderStatus::RESERVED));
    repo.create(makeOrder("S-002", "고객B", 20, OrderStatus::RESERVED));

    auto rejected = repo.findByStatus(OrderStatus::REJECTED);
    EXPECT_TRUE(rejected.empty());
}

// -----------------------------------------------------------------------
// TC-10: update() — RESERVED → CONFIRMED 상태 변경 후 findById() 반영 확인
// -----------------------------------------------------------------------
TEST_F(OrderRepositoryTest, Update_StatusChange_ReflectedInFindById)
{
    auto repo = makeRepo();
    std::string id = repo.create(makeOrder());

    auto order = repo.findById(id);
    ASSERT_TRUE(order.has_value());
    EXPECT_EQ(order->status, OrderStatus::RESERVED);

    order->status = OrderStatus::CONFIRMED;
    ASSERT_TRUE(repo.update(*order));

    auto updated = repo.findById(id);
    ASSERT_TRUE(updated.has_value());
    EXPECT_EQ(updated->status, OrderStatus::CONFIRMED);
}

// -----------------------------------------------------------------------
// TC-11: update() — 존재하지 않는 ID → false 반환
// -----------------------------------------------------------------------
TEST_F(OrderRepositoryTest, Update_NonExistentId_ReturnsFalse)
{
    auto repo = makeRepo();
    Order ghost = makeOrder();
    ghost.id = "ORD-999";
    EXPECT_FALSE(repo.update(ghost));
}
