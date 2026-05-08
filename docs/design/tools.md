# Design: 관리자 도구 (Tools)

> **관련 문서**
> - PRD: [docs/prd/tools.md](../prd/tools.md)
> - PLAN: [docs/plan/tools.md](../plan/tools.md)

> **선행 조건:** 모든 Repository 구현 완료 (JsonSampleRepository, JsonOrderRepository, JsonProductionRepository)

---

## Phase 1: DataMonitorTool 구현

### Tasks

#### 구현
- [ ] `Tools/DataMonitorTool.h/.cpp`
  - `run()`: 메인 메뉴 루프 — [1] 시료 조회 / [2] 주문 조회 / [3] 생산 조회 / [0] 종료
  - `sampleMenu()`:
    - [1] 전체 목록: ID / 이름 / 수율 / 평균생산시간 / 재고 테이블 출력
    - [2] ID 검색: `findById()`
    - [3] 이름 검색: `searchByName()`
  - `orderMenu()`:
    - [1] 전체 목록: 주문 ID / sampleId / 고객명 / 수량 / 상태 / 접수일시 테이블
    - [2] 상태별 필터: RESERVED / PRODUCING / CONFIRMED / RELEASE / REJECTED 선택
  - `productionMenu()`:
    - activeJob 정보 출력 (없으면 "생산 중 없음")
    - queue 목록 출력 (없으면 "대기 없음")
  - DataMonitor PoC 콘솔 헬퍼 패턴 적용: `getChoice()`, `pause()`, `separator()`, `toLower()`

- [ ] `main.cpp`에 `--dump-data` 인수 분기 추가
  ```cpp
  if (hasArg(argc, argv, "--dump-data")) {
      JsonSampleRepository     sampleRepo("data/samples.json");
      JsonOrderRepository      orderRepo("data/orders.json");
      JsonProductionRepository productionRepo("data/production.json");
      return DataMonitorTool(sampleRepo, orderRepo, productionRepo).run();
  }
  ```

### 검증
- `.\x64\Debug\SampleOrderSystem.exe --dump-data` 실행
- 세 JSON 파일의 현재 내용이 콘솔에 사람이 읽기 좋은 형식으로 출력되는지 확인
- 데이터 파일 없을 때 (빈 파일) → 에러 없이 "데이터 없음" 메시지 출력 확인

---

## Phase 2: SemiDummyGenerator 구현

### Tasks

#### 테스트 코드
- [ ] `test/SemiDummyGeneratorTest.cpp` 작성
  - 생성된 Order의 `sampleId`가 모두 생성된 Sample 목록에 존재하는지 확인 (BR-01)
  - 생성된 Sample의 `yieldRate`: 0 초과 1.0 이하 확인 (BR-14)
  - PRODUCING 상태 Order → production.json의 activeJob 또는 queue에 모두 포함 확인 (BR-04)
  - activeJob이 최대 1개인지 확인 (BR-13)
  - PRODUCING Order의 `actualProductionQty` = `ceil(부족분 / (yieldRate × 0.9))` 확인 (BR-06)
  - PRODUCING Order의 `totalProductionTimeMin` = `avgProductionTime × actualProductionQty` 확인 (BR-07)
  - queue 순서: 주문 생성 순서와 동일한 FIFO 순서 확인 (BR-08)

#### 구현
- [ ] `Tools/SemiDummyGenerator.h/.cpp`
  - `run(bool append)`: 생성 실행 (append=false면 덮어쓰기 전 확인 메시지)
  - `generateSamples()`: 5~10개 시료 생성
    - 고정 풀: `{"AlGaN", "GaN", "SiC", "InP", "GaAs", "InGaAs", "AlN", "Si", "Ge", "ZnO"}`
    - `avgProductionTime`: 3~30분 랜덤
    - `yieldRate`: 0.70~1.00 (소수점 2자리)
    - `currentStock`: 10~100 랜덤
  - `generateOrders(samples)`: 10~20개 주문 생성
    - `sampleId`: 생성된 samples 중 랜덤 선택
    - 상태 비율: CONFIRMED 50% / PRODUCING 30% / RESERVED 20%
    - CONFIRMED: `quantity <= sample.currentStock` 보장
    - PRODUCING: `quantity > sample.currentStock` + BR-06, BR-07 계산
  - `generateProduction(producingOrders)`: production.json 생성
    - `producingOrders[0]` → activeJob (`startTimeUnix` = 현재 - 랜덤 미완료 시간)
    - `producingOrders[1..]` → queue (FIFO)
  - DummyDataGenerator PoC의 랜덤 유틸리티 패턴 적용: `std::mt19937`, `uniform_int_distribution`, `uniform_real_distribution`

- [ ] `main.cpp`에 `--gen-dummy` / `--gen-dummy --append` 인수 분기 추가
  ```cpp
  if (hasArg(argc, argv, "--gen-dummy")) {
      bool append = hasArg(argc, argv, "--append");
      return SemiDummyGenerator().run(append);
  }
  ```

### 검증
- 모든 테스트 PASS
- `.\x64\Debug\SampleOrderSystem.exe --gen-dummy` 실행 후:
  - 세 JSON 파일 생성 확인
  - `--dump-data`로 생성 데이터 육안 확인
  - 메인 프로그램(`SampleOrderSystem.exe`) 정상 실행 + 시료/주문/생산 데이터 정상 로드 확인
- `.\x64\Debug\SampleOrderSystem.exe --gen-dummy --append` 실행 후 기존 데이터에 추가 확인
