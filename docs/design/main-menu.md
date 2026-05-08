# Design: 메인 메뉴 (Main Menu)

> **관련 문서**
> - PRD: [docs/prd/main-menu.md](../prd/main-menu.md)
> - PLAN: [docs/plan/main-menu.md](../plan/main-menu.md)

> **선행 조건:** 모든 기능 Design(sample-management, order, production-line, shipment, monitoring)의 모든 Phase 완료

---

## Phase 1: AppController 구현

### Tasks

#### 테스트 코드
- [ ] `test/AppControllerTest.cpp` 작성 (Mock Controller + Mock ProductionService 사용)
  - `run()` 루프 시작 시 `ProductionService::checkAndComplete()` 호출 확인 (Lazy 체크)
  - 입력 `1` → `SampleController` 진입 확인
  - 입력 `2` → `OrderController` 진입 확인
  - 입력 `3` → `MonitoringController` 진입 확인
  - 입력 `4` → `ShipmentController` 진입 확인
  - 입력 `5` → `ProductionLineController` 진입 확인
  - 입력 `0` → 루프 종료 확인
  - 범위 외 입력 → 에러 메시지 + 루프 계속 확인

#### 구현
- [ ] `Controller/AppController.h/.cpp`
  - 생성자: 모든 서브컨트롤러 레퍼런스 + `ProductionService` 레퍼런스 수신
  - `run()`:
    ```
    while (true):
        productionService_.checkAndComplete()
        view_.showMainMenu(sampleRepository_.findAll())
        choice = view_.getMenuChoice()
        switch(choice): 1~5 서브컨트롤러, 0 break
    ```

### 검증
- 모든 테스트 PASS

---

## Phase 2: MainMenuView 구현

### Tasks

- [ ] `View/MainMenuView.h/.cpp`
  - `showMainMenu(samples)`: 재고 현황 스냅샷 (전체 시료 ID / 이름 / 재고) + 메뉴 항목 출력
  - `getMenuChoice()`: 0~5 범위 입력 루프 (`getChoice(0, 5)` 패턴)
  - `showInvalidInput()`: 범위 외 입력 에러 메시지

### 검증
- 수동 실행: 메인 화면에 전체 시료 재고 현황 표시 확인
- 수동 실행: 범위 외 입력(예: 9, -1, 문자) → 에러 메시지 + 재입력 프롬프트 확인
- 수동 실행: `0` 입력 → 프로그램 정상 종료 확인

---

## Phase 3: main.cpp — Composition Root 구현

### Tasks

- [ ] `SampleOrderSystem/main.cpp` 구현
  ```
  main(argc, argv):
    // 인수 분기
    if "--dump-data" → DataMonitorTool().run(); return
    if "--gen-dummy" → SemiDummyGenerator().run(hasArg("--append")); return
    bool mockTime = hasArg("--mock-time")

    // 인프라 생성
    ITimeProvider* tp = mockTime ? new MockTimeProvider() : new RealTimeProvider()

    // Repository 생성 (싱글턴)
    JsonSampleRepository     sampleRepo("data/samples.json")
    JsonOrderRepository      orderRepo("data/orders.json")
    JsonProductionRepository productionRepo("data/production.json")

    // Service 생성
    ProductionService productionService(sampleRepo, orderRepo, productionRepo, *tp)

    // View 생성
    SampleView        sampleView
    OrderView         orderView
    ShipmentView      shipmentView
    MonitoringView    monitoringView
    ProductionLineView productionLineView(mockTime)
    MainMenuView      mainMenuView

    // Controller 생성
    SampleController        sampleCtrl(sampleRepo, sampleView)
    OrderController         orderCtrl(orderRepo, sampleRepo, productionRepo, *tp, orderView)
    ShipmentController      shipmentCtrl(orderRepo, sampleRepo, shipmentView)
    MonitoringController    monitoringCtrl(orderRepo, sampleRepo, monitoringView)
    ProductionLineController productionLineCtrl(productionService, *tp, productionLineView)

    AppController appCtrl(sampleCtrl, orderCtrl, shipmentCtrl, monitoringCtrl,
                          productionLineCtrl, productionService, sampleRepo, mainMenuView)
    appCtrl.run()
  ```
- [ ] Windows 콘솔 UTF-8 설정: `SetConsoleCP(CP_UTF8)` / `SetConsoleOutputCP(CP_UTF8)`

### 검증
- 빌드 성공 (`msbuild SampleOrderSystem.slnx /p:Configuration=Debug /p:Platform=x64`)
- `.\x64\Debug\SampleOrderSystem.exe` 실행 → 메인 메뉴 정상 표시

---

## Phase 4: 전체 통합 시나리오 검증

### Tasks

- [ ] E2E 시나리오 수동 실행 (정상 흐름)
  1. `--gen-dummy`로 초기 데이터 생성
  2. 메인 프로그램 실행 → 시료 관리 → 시료 등록
  3. 주문 접수 → 재고 부족 승인 → PRODUCING 전환 확인
  4. `--mock-time` 모드 재실행 → 시간 앞당기기 → 생산 완료 자동 처리 확인
  5. 출고 처리 → RELEASE 전환 + 재고 차감 확인
  6. 모니터링 → 상태별 집계 + 재고 상태 확인

- [ ] E2E 시나리오 수동 실행 (재시작 내구성)
  1. 주문 승인 → PRODUCING 상태에서 앱 종료
  2. 앱 재시작 (충분한 시간 경과 후) → 메인 메뉴 진입 시 생산 완료 자동 처리 확인

### 검증
- 위 시나리오가 에러 없이 모두 통과
- `data/` 폴더의 세 JSON 파일이 PRD 데이터 모델 형식을 유지하는지 최종 확인
