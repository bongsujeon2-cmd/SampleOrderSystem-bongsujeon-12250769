// SampleRepositoryTest.cpp
// JsonSampleRepository 단위 테스트
// 구현 코드 없이 작성된 TDD 테스트 파일 (테스트는 구현 전이므로 FAIL 상태가 정상)

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Model/Repository/JsonSampleRepository.h"

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// -----------------------------------------------------------------------
// 헬퍼
// -----------------------------------------------------------------------

/// 테스트용 Sample 객체 생성 헬퍼
static Sample makeSample(
    const std::string& id,
    const std::string& name,
    int    avgProductionTime = 5,
    double yieldRate         = 0.9,
    int    currentStock      = 100)
{
    Sample s;
    s.id                = id;
    s.name              = name;
    s.avgProductionTime = avgProductionTime;
    s.yieldRate         = yieldRate;
    s.currentStock      = currentStock;
    return s;
}

// -----------------------------------------------------------------------
// Fixture
// -----------------------------------------------------------------------

class SampleRepositoryTest : public ::testing::Test
{
protected:
    std::string tempFilePath;

    void SetUp() override
    {
        // 각 테스트마다 고유한 임시 파일 경로 생성
        // GetCurrentTestInfo()로 테스트 이름을 포함시켜 중복을 방지한다.
        const ::testing::TestInfo* info =
            ::testing::UnitTest::GetInstance()->current_test_info();
        tempFilePath = (fs::temp_directory_path() /
            (std::string("test_samples_") + info->test_suite_name() + "_" + info->name() + ".json")
        ).string();
    }

    void TearDown() override
    {
        // 테스트 종료 후 임시 파일 삭제
        if (fs::exists(tempFilePath))
        {
            fs::remove(tempFilePath);
        }
    }

    /// 매 테스트에서 신선한 Repository 인스턴스를 생성한다.
    JsonSampleRepository makeRepo() const
    {
        return JsonSampleRepository(tempFilePath);
    }
};

// -----------------------------------------------------------------------
// TC-01: save() 후 findById()로 조회 성공 — 모든 필드 검증
// -----------------------------------------------------------------------
TEST_F(SampleRepositoryTest, SaveAndFindById_AllFieldsMatch)
{
    auto repo   = makeRepo();
    Sample s    = makeSample("S-001", "AlGaN", 5, 0.9, 50);
    ASSERT_TRUE(repo.save(s));

    auto result = repo.findById("S-001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->id,                "S-001");
    EXPECT_EQ(result->name,              "AlGaN");
    EXPECT_EQ(result->avgProductionTime, 5);
    EXPECT_DOUBLE_EQ(result->yieldRate,  0.9);
    EXPECT_EQ(result->currentStock,      50);
}

// -----------------------------------------------------------------------
// TC-02: findById() — 없는 ID → nullopt 반환
// -----------------------------------------------------------------------
TEST_F(SampleRepositoryTest, FindById_NotFound_ReturnsNullopt)
{
    auto repo   = makeRepo();
    auto result = repo.findById("NONEXISTENT");
    EXPECT_FALSE(result.has_value());
}

// -----------------------------------------------------------------------
// TC-03: existsId() — 저장 후 true, 없는 ID → false
// -----------------------------------------------------------------------
TEST_F(SampleRepositoryTest, ExistsId_AfterSave_TrueForExistingFalseForMissing)
{
    auto repo = makeRepo();
    repo.save(makeSample("S-001", "AlGaN"));

    EXPECT_TRUE(repo.existsId("S-001"));
    EXPECT_FALSE(repo.existsId("S-999"));
}

// -----------------------------------------------------------------------
// TC-04: existsName() — 저장 후 true, 없는 이름 → false
// -----------------------------------------------------------------------
TEST_F(SampleRepositoryTest, ExistsName_AfterSave_TrueForExistingFalseForMissing)
{
    auto repo = makeRepo();
    repo.save(makeSample("S-001", "AlGaN"));

    EXPECT_TRUE(repo.existsName("AlGaN"));
    EXPECT_FALSE(repo.existsName("Unknown"));
}

// -----------------------------------------------------------------------
// TC-05: findAll() — 3개 저장 후 크기 3
// -----------------------------------------------------------------------
TEST_F(SampleRepositoryTest, FindAll_ThreeSamples_ReturnsThreeItems)
{
    auto repo = makeRepo();
    repo.save(makeSample("S-001", "AlGaN"));
    repo.save(makeSample("S-002", "GaN"));
    repo.save(makeSample("S-003", "SiC"));

    auto all = repo.findAll();
    EXPECT_EQ(all.size(), 3u);
}

// -----------------------------------------------------------------------
// TC-06: findAll() — 빈 저장소 → 빈 벡터
// -----------------------------------------------------------------------
TEST_F(SampleRepositoryTest, FindAll_EmptyRepository_ReturnsEmptyVector)
{
    auto repo = makeRepo();
    auto all  = repo.findAll();
    EXPECT_TRUE(all.empty());
}

// -----------------------------------------------------------------------
// TC-07: searchByName("GaN") — 부분 일치: "AlGaN", "GaN" 둘 다 매칭
// -----------------------------------------------------------------------
TEST_F(SampleRepositoryTest, SearchByName_PartialMatch_ReturnsBothMatches)
{
    auto repo = makeRepo();
    repo.save(makeSample("S-001", "AlGaN"));
    repo.save(makeSample("S-002", "GaN"));
    repo.save(makeSample("S-003", "SiC"));

    auto results = repo.searchByName("GaN");
    ASSERT_EQ(results.size(), 2u);

    // 반환 순서는 구현에 따라 다를 수 있으므로 집합으로 확인
    std::vector<std::string> names;
    for (const auto& r : results)
        names.push_back(r.name);

    EXPECT_THAT(names, ::testing::UnorderedElementsAre("AlGaN", "GaN"));
}

// -----------------------------------------------------------------------
// TC-08: searchByName — 대소문자 무시: "algan" → "AlGaN" 매칭
// -----------------------------------------------------------------------
TEST_F(SampleRepositoryTest, SearchByName_CaseInsensitive_MatchesAlGaN)
{
    auto repo = makeRepo();
    repo.save(makeSample("S-001", "AlGaN"));
    repo.save(makeSample("S-002", "SiC"));

    auto results = repo.searchByName("algan");
    ASSERT_EQ(results.size(), 1u);
    EXPECT_EQ(results[0].name, "AlGaN");
}

// -----------------------------------------------------------------------
// TC-09: searchByName — 없는 키워드 → 빈 벡터
// -----------------------------------------------------------------------
TEST_F(SampleRepositoryTest, SearchByName_NoMatch_ReturnsEmptyVector)
{
    auto repo = makeRepo();
    repo.save(makeSample("S-001", "AlGaN"));
    repo.save(makeSample("S-002", "GaN"));

    auto results = repo.searchByName("InP");
    EXPECT_TRUE(results.empty());
}

// -----------------------------------------------------------------------
// TC-10: update() — 재고 수량 변경 후 findById()로 반영 확인
// -----------------------------------------------------------------------
TEST_F(SampleRepositoryTest, Update_StockChange_ReflectedInFindById)
{
    auto repo = makeRepo();
    repo.save(makeSample("S-001", "AlGaN", 5, 0.9, 100));

    Sample updated       = makeSample("S-001", "AlGaN", 5, 0.9, 200);
    ASSERT_TRUE(repo.update(updated));

    auto result = repo.findById("S-001");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->currentStock, 200);
}

// -----------------------------------------------------------------------
// TC-11: update() — 없는 ID → false 반환
// -----------------------------------------------------------------------
TEST_F(SampleRepositoryTest, Update_NonExistentId_ReturnsFalse)
{
    auto repo = makeRepo();
    Sample phantom = makeSample("GHOST", "Phantom");
    EXPECT_FALSE(repo.update(phantom));
}

// -----------------------------------------------------------------------
// TC-12: 영속성 — Repository 파괴 후 재생성 → 데이터 유지
// -----------------------------------------------------------------------
TEST_F(SampleRepositoryTest, Persistence_AfterReload_DataIsPreserved)
{
    // 첫 번째 Repository 인스턴스: 데이터 저장 후 소멸
    {
        JsonSampleRepository repo(tempFilePath);
        repo.save(makeSample("S-001", "AlGaN", 5, 0.9, 50));
        repo.save(makeSample("S-002", "GaN",   3, 0.85, 30));
    }

    // 두 번째 Repository 인스턴스: 같은 파일로 재생성
    {
        JsonSampleRepository repo(tempFilePath);

        auto all = repo.findAll();
        EXPECT_EQ(all.size(), 2u);

        auto s1 = repo.findById("S-001");
        ASSERT_TRUE(s1.has_value());
        EXPECT_EQ(s1->name,              "AlGaN");
        EXPECT_EQ(s1->avgProductionTime, 5);
        EXPECT_DOUBLE_EQ(s1->yieldRate,  0.9);
        EXPECT_EQ(s1->currentStock,      50);

        auto s2 = repo.findById("S-002");
        ASSERT_TRUE(s2.has_value());
        EXPECT_EQ(s2->name,              "GaN");
        EXPECT_EQ(s2->avgProductionTime, 3);
        EXPECT_DOUBLE_EQ(s2->yieldRate,  0.85);
        EXPECT_EQ(s2->currentStock,      30);
    }
}
