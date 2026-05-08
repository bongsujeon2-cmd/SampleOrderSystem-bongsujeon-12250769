// SampleControllerTest.cpp
// SampleController 단위 테스트 (TDD — 구현 없이 먼저 작성)
// 이 시점에서 구현이 없으므로 테스트는 FAIL 상태가 정상

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Model/Repository/ISampleRepository.h"
#include "../View/ISampleView.h"
#include "../Controller/SampleController.h"

using ::testing::Return;
using ::testing::_;
using ::testing::Eq;

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

class MockSampleView : public ISampleView {
public:
    MOCK_METHOD(void, showSubMenu, (), (override));
    MOCK_METHOD(int, getSubMenuChoice, (), (override));
    MOCK_METHOD(Sample, promptSampleInput, (), (override));
    MOCK_METHOD(void, showSampleList, (const std::vector<Sample>&), (override));
    MOCK_METHOD(void, showSearchResult, (const std::vector<Sample>&), (override));
    MOCK_METHOD(void, showError, (const std::string&), (override));
    MOCK_METHOD(void, showSuccess, (const std::string&), (override));
    MOCK_METHOD(std::string, promptKeyword, (), (override));
};

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

class SampleControllerTest : public ::testing::Test {
protected:
    MockSampleRepository mockRepo;
    MockSampleView       mockView;

    // 각 테스트에서 독립적인 SampleController 인스턴스를 생성하는 헬퍼
    SampleController makeController() {
        return SampleController(mockRepo, mockView);
    }
};

// -----------------------------------------------------------------------
// TC-01: registerSample() — 정상 등록
// existsId=false, existsName=false, 유효 yieldRate(0.9)
// → repo.save() 1회 호출, view.showSuccess() 1회 호출
// -----------------------------------------------------------------------
TEST_F(SampleControllerTest, RegisterSample_ValidInput_SavesAndShowsSuccess)
{
    Sample input = makeSample("S-001", "AlGaN", 5, 0.9, 100);

    EXPECT_CALL(mockView, promptSampleInput())
        .Times(1)
        .WillOnce(Return(input));

    EXPECT_CALL(mockRepo, existsId(Eq(std::string("S-001"))))
        .Times(1)
        .WillOnce(Return(false));

    EXPECT_CALL(mockRepo, existsName(Eq(std::string("AlGaN"))))
        .Times(1)
        .WillOnce(Return(false));

    EXPECT_CALL(mockRepo, save(_))
        .Times(1)
        .WillOnce(Return(true));

    EXPECT_CALL(mockView, showSuccess(_))
        .Times(1);

    // showError()는 절대 호출되어서는 안 됨
    EXPECT_CALL(mockView, showError(_))
        .Times(0);

    auto controller = makeController();
    controller.registerSample();
}

// -----------------------------------------------------------------------
// TC-02: registerSample() — 중복 ID
// existsId=true → view.showError() 1회 호출, repo.save() 미호출
// -----------------------------------------------------------------------
TEST_F(SampleControllerTest, RegisterSample_DuplicateId_ShowsErrorAndDoesNotSave)
{
    Sample input = makeSample("S-001", "AlGaN", 5, 0.9, 100);

    EXPECT_CALL(mockView, promptSampleInput())
        .Times(1)
        .WillOnce(Return(input));

    EXPECT_CALL(mockRepo, existsId(Eq(std::string("S-001"))))
        .Times(1)
        .WillOnce(Return(true));

    // existsId가 true이므로 existsName은 호출되어서는 안 됨
    EXPECT_CALL(mockRepo, existsName(_))
        .Times(0);

    // save()는 절대 호출되어서는 안 됨
    EXPECT_CALL(mockRepo, save(_))
        .Times(0);

    EXPECT_CALL(mockView, showError(_))
        .Times(1);

    EXPECT_CALL(mockView, showSuccess(_))
        .Times(0);

    auto controller = makeController();
    controller.registerSample();
}

// -----------------------------------------------------------------------
// TC-03: registerSample() — 중복 이름
// existsId=false, existsName=true
// → view.showError() 1회 호출, repo.save() 미호출
// -----------------------------------------------------------------------
TEST_F(SampleControllerTest, RegisterSample_DuplicateName_ShowsErrorAndDoesNotSave)
{
    Sample input = makeSample("S-002", "AlGaN", 5, 0.9, 100);

    EXPECT_CALL(mockView, promptSampleInput())
        .Times(1)
        .WillOnce(Return(input));

    EXPECT_CALL(mockRepo, existsId(Eq(std::string("S-002"))))
        .Times(1)
        .WillOnce(Return(false));

    EXPECT_CALL(mockRepo, existsName(Eq(std::string("AlGaN"))))
        .Times(1)
        .WillOnce(Return(true));

    // save()는 절대 호출되어서는 안 됨
    EXPECT_CALL(mockRepo, save(_))
        .Times(0);

    EXPECT_CALL(mockView, showError(_))
        .Times(1);

    EXPECT_CALL(mockView, showSuccess(_))
        .Times(0);

    auto controller = makeController();
    controller.registerSample();
}

// -----------------------------------------------------------------------
// TC-04: registerSample() — 수율 0.0 (경계값: 0 이하)
// yieldRate=0.0 → view.showError() 1회 호출, repo.save() 미호출
// -----------------------------------------------------------------------
TEST_F(SampleControllerTest, RegisterSample_YieldRateZero_ShowsErrorAndDoesNotSave)
{
    Sample input = makeSample("S-003", "GaN", 5, 0.0, 100);

    EXPECT_CALL(mockView, promptSampleInput())
        .Times(1)
        .WillOnce(Return(input));

    EXPECT_CALL(mockRepo, existsId(Eq(std::string("S-003"))))
        .Times(1)
        .WillOnce(Return(false));

    EXPECT_CALL(mockRepo, existsName(Eq(std::string("GaN"))))
        .Times(1)
        .WillOnce(Return(false));

    // save()는 절대 호출되어서는 안 됨
    EXPECT_CALL(mockRepo, save(_))
        .Times(0);

    EXPECT_CALL(mockView, showError(_))
        .Times(1);

    EXPECT_CALL(mockView, showSuccess(_))
        .Times(0);

    auto controller = makeController();
    controller.registerSample();
}

// -----------------------------------------------------------------------
// TC-05: registerSample() — 수율 1.1 (경계값: 1.0 초과)
// yieldRate=1.1 → view.showError() 1회 호출, repo.save() 미호출
// -----------------------------------------------------------------------
TEST_F(SampleControllerTest, RegisterSample_YieldRateExceedsOne_ShowsErrorAndDoesNotSave)
{
    Sample input = makeSample("S-004", "SiC", 5, 1.1, 100);

    EXPECT_CALL(mockView, promptSampleInput())
        .Times(1)
        .WillOnce(Return(input));

    EXPECT_CALL(mockRepo, existsId(Eq(std::string("S-004"))))
        .Times(1)
        .WillOnce(Return(false));

    EXPECT_CALL(mockRepo, existsName(Eq(std::string("SiC"))))
        .Times(1)
        .WillOnce(Return(false));

    // save()는 절대 호출되어서는 안 됨
    EXPECT_CALL(mockRepo, save(_))
        .Times(0);

    EXPECT_CALL(mockView, showError(_))
        .Times(1);

    EXPECT_CALL(mockView, showSuccess(_))
        .Times(0);

    auto controller = makeController();
    controller.registerSample();
}

// -----------------------------------------------------------------------
// TC-06: listSamples() — repo.findAll() 반환 결과가 view.showSampleList()에 전달됨
// -----------------------------------------------------------------------
TEST_F(SampleControllerTest, ListSamples_FindAllResult_PassedToShowSampleList)
{
    std::vector<Sample> samples = {
        makeSample("S-001", "AlGaN", 5, 0.9, 100),
        makeSample("S-002", "GaN",   3, 0.85, 50),
        makeSample("S-003", "SiC",   7, 0.95, 200),
    };

    EXPECT_CALL(mockRepo, findAll())
        .Times(1)
        .WillOnce(Return(samples));

    // showSampleList에 repo.findAll()의 반환값이 그대로 전달되어야 함
    EXPECT_CALL(mockView, showSampleList(Eq(samples)))
        .Times(1);

    auto controller = makeController();
    controller.listSamples();
}

// -----------------------------------------------------------------------
// TC-07: searchSamples() — view.promptKeyword() 반환값으로 repo.searchByName() 호출,
//         결과가 view.showSearchResult()에 전달됨
// -----------------------------------------------------------------------
TEST_F(SampleControllerTest, SearchSamples_KeywordFromView_PassedToRepoAndResultToView)
{
    const std::string keyword = "GaN";
    std::vector<Sample> results = {
        makeSample("S-001", "AlGaN", 5, 0.9, 100),
        makeSample("S-002", "GaN",   3, 0.85, 50),
    };

    EXPECT_CALL(mockView, promptKeyword())
        .Times(1)
        .WillOnce(Return(keyword));

    EXPECT_CALL(mockRepo, searchByName(Eq(keyword)))
        .Times(1)
        .WillOnce(Return(results));

    // searchByName의 반환값이 showSearchResult에 그대로 전달되어야 함
    EXPECT_CALL(mockView, showSearchResult(Eq(results)))
        .Times(1);

    auto controller = makeController();
    controller.searchSamples();
}

// -----------------------------------------------------------------------
// TC-08: registerSample() — avgProductionTime=0 (경계값: 0 이하)
// avgProductionTime=0 → view.showError() 1회 호출, repo.save() 미호출
// -----------------------------------------------------------------------
TEST_F(SampleControllerTest, RegisterSample_AvgTimZero_ShowsErrorAndDoesNotSave)
{
    Sample input = makeSample("S-005", "InP", 0, 0.9, 0);

    EXPECT_CALL(mockView, promptSampleInput())
        .Times(1)
        .WillOnce(Return(input));

    EXPECT_CALL(mockRepo, existsId(Eq(std::string("S-005"))))
        .Times(1)
        .WillOnce(Return(false));

    EXPECT_CALL(mockRepo, existsName(Eq(std::string("InP"))))
        .Times(1)
        .WillOnce(Return(false));

    // save()는 절대 호출되어서는 안 됨
    EXPECT_CALL(mockRepo, save(_))
        .Times(0);

    EXPECT_CALL(mockView, showError(_))
        .Times(1);

    EXPECT_CALL(mockView, showSuccess(_))
        .Times(0);

    auto controller = makeController();
    controller.registerSample();
}
