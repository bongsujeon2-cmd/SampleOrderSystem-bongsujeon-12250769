// AppControllerTest.cpp
// AppController 단위 테스트 (TDD — 구현 없이 먼저 작성)
// 이 시점에서 구현이 없으므로 테스트는 FAIL 상태가 정상

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "../Model/Service/IProductionService.h"
#include "../Model/Repository/ISampleRepository.h"
#include "../Model/Domain/Sample.h"
#include "../View/IMainMenuView.h"
#include "../Controller/ISubController.h"
#include "../Controller/AppController.h"

using ::testing::Return;
using ::testing::_;
using ::testing::Eq;
using ::testing::InSequence;

// -----------------------------------------------------------------------
// Mock 클래스
// -----------------------------------------------------------------------

class MockSubController : public ISubController {
public:
    MOCK_METHOD(void, run, (), (override));
};

class MockProductionService : public IProductionService {
public:
    MOCK_METHOD(void, checkAndComplete, (), (override));
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

class MockMainMenuView : public IMainMenuView {
public:
    MOCK_METHOD(void, showMainMenu, (const std::vector<Sample>&), (override));
    MOCK_METHOD(int,  getMenuChoice, (), (override));
    MOCK_METHOD(void, showInvalidInput, (), (override));
};

// -----------------------------------------------------------------------
// Fixture
// -----------------------------------------------------------------------

class AppControllerTest : public ::testing::Test {
protected:
    MockSubController       mockSampleCtrl;
    MockSubController       mockOrderCtrl;
    MockSubController       mockMonitoringCtrl;
    MockSubController       mockShipmentCtrl;
    MockSubController       mockProductionLineCtrl;
    MockProductionService   mockProductionService;
    MockSampleRepository    mockSampleRepo;
    MockMainMenuView        mockView;

    AppController makeController() {
        return AppController(
            mockSampleCtrl,
            mockOrderCtrl,
            mockMonitoringCtrl,
            mockShipmentCtrl,
            mockProductionLineCtrl,
            mockProductionService,
            mockSampleRepo,
            mockView
        );
    }
};

// -----------------------------------------------------------------------
// TC-01: Run_CheckAndCompleteCalledEachLoopIteration
// getMenuChoice() → 9, 0 순서 (invalid → exit, 루프 2회 순환)
// checkAndComplete() 2회 호출, showInvalidInput() 1회 호출 확인
// -----------------------------------------------------------------------
TEST_F(AppControllerTest, Run_CheckAndCompleteCalledEachLoopIteration)
{
    EXPECT_CALL(mockProductionService, checkAndComplete())
        .Times(2);

    EXPECT_CALL(mockSampleRepo, findAll())
        .WillRepeatedly(Return(std::vector<Sample>{}));

    EXPECT_CALL(mockView, showMainMenu(_))
        .WillRepeatedly(Return());

    EXPECT_CALL(mockView, getMenuChoice())
        .WillOnce(Return(9))
        .WillOnce(Return(0));

    EXPECT_CALL(mockView, showInvalidInput())
        .Times(1);

    auto controller = makeController();
    controller.run();
}

// -----------------------------------------------------------------------
// TC-02: Run_Choice1_CallsSampleControllerRun
// getMenuChoice() → 1, 0
// sampleCtrl.run() 1회 호출
// -----------------------------------------------------------------------
TEST_F(AppControllerTest, Run_Choice1_CallsSampleControllerRun)
{
    EXPECT_CALL(mockProductionService, checkAndComplete())
        .WillRepeatedly(Return());

    EXPECT_CALL(mockSampleRepo, findAll())
        .WillRepeatedly(Return(std::vector<Sample>{}));

    EXPECT_CALL(mockView, showMainMenu(_))
        .WillRepeatedly(Return());

    EXPECT_CALL(mockView, getMenuChoice())
        .WillOnce(Return(1))
        .WillOnce(Return(0));

    EXPECT_CALL(mockSampleCtrl, run())
        .Times(1);

    EXPECT_CALL(mockOrderCtrl, run()).Times(0);
    EXPECT_CALL(mockMonitoringCtrl, run()).Times(0);
    EXPECT_CALL(mockShipmentCtrl, run()).Times(0);
    EXPECT_CALL(mockProductionLineCtrl, run()).Times(0);

    auto controller = makeController();
    controller.run();
}

// -----------------------------------------------------------------------
// TC-03: Run_Choice2_CallsOrderControllerRun
// getMenuChoice() → 2, 0
// orderCtrl.run() 1회 호출
// -----------------------------------------------------------------------
TEST_F(AppControllerTest, Run_Choice2_CallsOrderControllerRun)
{
    EXPECT_CALL(mockProductionService, checkAndComplete())
        .WillRepeatedly(Return());

    EXPECT_CALL(mockSampleRepo, findAll())
        .WillRepeatedly(Return(std::vector<Sample>{}));

    EXPECT_CALL(mockView, showMainMenu(_))
        .WillRepeatedly(Return());

    EXPECT_CALL(mockView, getMenuChoice())
        .WillOnce(Return(2))
        .WillOnce(Return(0));

    EXPECT_CALL(mockSampleCtrl, run()).Times(0);
    EXPECT_CALL(mockOrderCtrl, run()).Times(1);
    EXPECT_CALL(mockMonitoringCtrl, run()).Times(0);
    EXPECT_CALL(mockShipmentCtrl, run()).Times(0);
    EXPECT_CALL(mockProductionLineCtrl, run()).Times(0);

    auto controller = makeController();
    controller.run();
}

// -----------------------------------------------------------------------
// TC-04: Run_Choice3_CallsMonitoringControllerRun
// getMenuChoice() → 3, 0
// monitoringCtrl.run() 1회 호출
// -----------------------------------------------------------------------
TEST_F(AppControllerTest, Run_Choice3_CallsMonitoringControllerRun)
{
    EXPECT_CALL(mockProductionService, checkAndComplete())
        .WillRepeatedly(Return());

    EXPECT_CALL(mockSampleRepo, findAll())
        .WillRepeatedly(Return(std::vector<Sample>{}));

    EXPECT_CALL(mockView, showMainMenu(_))
        .WillRepeatedly(Return());

    EXPECT_CALL(mockView, getMenuChoice())
        .WillOnce(Return(3))
        .WillOnce(Return(0));

    EXPECT_CALL(mockSampleCtrl, run()).Times(0);
    EXPECT_CALL(mockOrderCtrl, run()).Times(0);
    EXPECT_CALL(mockMonitoringCtrl, run()).Times(1);
    EXPECT_CALL(mockShipmentCtrl, run()).Times(0);
    EXPECT_CALL(mockProductionLineCtrl, run()).Times(0);

    auto controller = makeController();
    controller.run();
}

// -----------------------------------------------------------------------
// TC-05: Run_Choice4_CallsShipmentControllerRun
// getMenuChoice() → 4, 0
// shipmentCtrl.run() 1회 호출
// -----------------------------------------------------------------------
TEST_F(AppControllerTest, Run_Choice4_CallsShipmentControllerRun)
{
    EXPECT_CALL(mockProductionService, checkAndComplete())
        .WillRepeatedly(Return());

    EXPECT_CALL(mockSampleRepo, findAll())
        .WillRepeatedly(Return(std::vector<Sample>{}));

    EXPECT_CALL(mockView, showMainMenu(_))
        .WillRepeatedly(Return());

    EXPECT_CALL(mockView, getMenuChoice())
        .WillOnce(Return(4))
        .WillOnce(Return(0));

    EXPECT_CALL(mockSampleCtrl, run()).Times(0);
    EXPECT_CALL(mockOrderCtrl, run()).Times(0);
    EXPECT_CALL(mockMonitoringCtrl, run()).Times(0);
    EXPECT_CALL(mockShipmentCtrl, run()).Times(1);
    EXPECT_CALL(mockProductionLineCtrl, run()).Times(0);

    auto controller = makeController();
    controller.run();
}

// -----------------------------------------------------------------------
// TC-06: Run_Choice5_CallsProductionLineControllerRun
// getMenuChoice() → 5, 0
// productionLineCtrl.run() 1회 호출
// -----------------------------------------------------------------------
TEST_F(AppControllerTest, Run_Choice5_CallsProductionLineControllerRun)
{
    EXPECT_CALL(mockProductionService, checkAndComplete())
        .WillRepeatedly(Return());

    EXPECT_CALL(mockSampleRepo, findAll())
        .WillRepeatedly(Return(std::vector<Sample>{}));

    EXPECT_CALL(mockView, showMainMenu(_))
        .WillRepeatedly(Return());

    EXPECT_CALL(mockView, getMenuChoice())
        .WillOnce(Return(5))
        .WillOnce(Return(0));

    EXPECT_CALL(mockSampleCtrl, run()).Times(0);
    EXPECT_CALL(mockOrderCtrl, run()).Times(0);
    EXPECT_CALL(mockMonitoringCtrl, run()).Times(0);
    EXPECT_CALL(mockShipmentCtrl, run()).Times(0);
    EXPECT_CALL(mockProductionLineCtrl, run()).Times(1);

    auto controller = makeController();
    controller.run();
}

// -----------------------------------------------------------------------
// TC-07: Run_Choice0_Exits
// getMenuChoice() → 0
// checkAndComplete() 1회, 어느 서브컨트롤러 run()도 미호출
// -----------------------------------------------------------------------
TEST_F(AppControllerTest, Run_Choice0_Exits)
{
    EXPECT_CALL(mockProductionService, checkAndComplete())
        .Times(1);

    EXPECT_CALL(mockSampleRepo, findAll())
        .Times(1)
        .WillOnce(Return(std::vector<Sample>{}));

    EXPECT_CALL(mockView, showMainMenu(_))
        .Times(1);

    EXPECT_CALL(mockView, getMenuChoice())
        .Times(1)
        .WillOnce(Return(0));

    EXPECT_CALL(mockSampleCtrl, run()).Times(0);
    EXPECT_CALL(mockOrderCtrl, run()).Times(0);
    EXPECT_CALL(mockMonitoringCtrl, run()).Times(0);
    EXPECT_CALL(mockShipmentCtrl, run()).Times(0);
    EXPECT_CALL(mockProductionLineCtrl, run()).Times(0);

    auto controller = makeController();
    controller.run();
}

// -----------------------------------------------------------------------
// TC-08: Run_InvalidChoice_ShowsInvalidInput
// getMenuChoice() → 9, 0
// showInvalidInput() 1회 호출, 어느 서브컨트롤러 run()도 미호출
// -----------------------------------------------------------------------
TEST_F(AppControllerTest, Run_InvalidChoice_ShowsInvalidInput)
{
    EXPECT_CALL(mockProductionService, checkAndComplete())
        .WillRepeatedly(Return());

    EXPECT_CALL(mockSampleRepo, findAll())
        .WillRepeatedly(Return(std::vector<Sample>{}));

    EXPECT_CALL(mockView, showMainMenu(_))
        .WillRepeatedly(Return());

    EXPECT_CALL(mockView, getMenuChoice())
        .WillOnce(Return(9))
        .WillOnce(Return(0));

    EXPECT_CALL(mockView, showInvalidInput())
        .Times(1);

    EXPECT_CALL(mockSampleCtrl, run()).Times(0);
    EXPECT_CALL(mockOrderCtrl, run()).Times(0);
    EXPECT_CALL(mockMonitoringCtrl, run()).Times(0);
    EXPECT_CALL(mockShipmentCtrl, run()).Times(0);
    EXPECT_CALL(mockProductionLineCtrl, run()).Times(0);

    auto controller = makeController();
    controller.run();
}

// -----------------------------------------------------------------------
// TC-09: Run_ShowsStockSnapshotEachLoop
// findAll() → samples 반환
// showMainMenu(samples) 호출 시 동일 벡터 전달 확인
// -----------------------------------------------------------------------
TEST_F(AppControllerTest, Run_ShowsStockSnapshotEachLoop)
{
    Sample s1, s2;
    s1.id = "S-001"; s1.name = "SampleA"; s1.avgProductionTime = 10; s1.yieldRate = 0.9; s1.currentStock = 50;
    s2.id = "S-002"; s2.name = "SampleB"; s2.avgProductionTime = 20; s2.yieldRate = 0.8; s2.currentStock = 30;
    std::vector<Sample> expectedSamples = { s1, s2 };

    EXPECT_CALL(mockProductionService, checkAndComplete())
        .WillRepeatedly(Return());

    EXPECT_CALL(mockSampleRepo, findAll())
        .WillRepeatedly(Return(expectedSamples));

    std::vector<Sample> capturedSamples;
    EXPECT_CALL(mockView, showMainMenu(_))
        .WillRepeatedly([&](const std::vector<Sample>& samples) {
            capturedSamples = samples;
        });

    EXPECT_CALL(mockView, getMenuChoice())
        .WillOnce(Return(0));

    auto controller = makeController();
    controller.run();

    ASSERT_EQ(capturedSamples.size(), 2u);
    EXPECT_EQ(capturedSamples[0], expectedSamples[0]);
    EXPECT_EQ(capturedSamples[1], expectedSamples[1]);
}
