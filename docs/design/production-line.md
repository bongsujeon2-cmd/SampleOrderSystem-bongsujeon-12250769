# Design: 생산 라인 (Production Line)

> **관련 문서**
> - PRD: [docs/prd/production-line.md](../prd/production-line.md)
> - PLAN: [docs/plan/production-line.md](../plan/production-line.md)

> **선행 조건:** `order` Design의 모든 Phase 완료 (IProductionRepository 인터페이스 + 주문 승인 시 enqueue 동작)

---

## Phase 1: JsonProductionRepository 구현

### Tasks

#### 테스트 코드
- [ ] `test/ProductionRepositoryTest.cpp` 작성
  - 빈 파일에서 `getState()` → activeJob = nullopt, queue 비어 있음 확인
  - `enqueue(job)` 후 `getState().queue` 크기 +1 확인
  - `enqueue()` 두 번 → queue FIFO 순서 유지 확인 (BR-08)
  - `setState()` 후 Repository 재생성 → 데이터 유지 확인 (영속성)
  - activeJob 있는 상태 → `setState()` → 재로드 후 동일 데이터 확인

#### 구현
- [ ] `Model/Repository/JsonProductionRepository.h/.cpp`
  - 생성자에서 `data/production.json` 로드 → 인메모리 `ProductionState` 유지
  - `getState()`: 인메모리 상태 반환
  - `setState(state)`: 인메모리 갱신 + 파일 전체 덮어쓰기 flush
  - `enqueue(job)`: `state.queue.push_back(job)` + `setState()` 호출

### 검증
- 모든 테스트 PASS
- `data/production.json` 파일이 PRD 스키마 형식(`activeJob` + `queue`)과 일치하는지 육안 확인

---

## Phase 2: ProductionService 구현

### Tasks

#### 테스트 코드
- [ ] `test/ProductionServiceTest.cpp` 작성 (MockTimeProvider + Mock 레포지토리 사용)

  **미완료 케이스**
  - `elapsed < totalProductionTimeMin × 60` → 아무 작업도 하지 않음 확인

  **완료 케이스 (BR-09)**
  - `elapsed >= totalProductionTimeMin × 60` → `ISampleRepository::update()` 호출 확인
  - 재고 증가량 = `actualProductionQty` 전량 확인
  - `IOrderRepository::update()` 호출, `order.status = CONFIRMED` 확인
  - `IProductionRepository::setState()` 호출, `activeJob = nullopt` 확인

  **Queue 자동 시작 (BR-08)**
  - 완료 후 queue에 대기 항목 있음 → `startNextJob()` 호출 확인
  - 다음 작업의 `startTimeUnix`에 `ITimeProvider::now()` 값 설정 확인

  **재귀 체크 (BR-17)**
  - MockTimeProvider로 큰 시간 설정 → queue의 모든 작업이 연쇄 완료되는지 확인

  **재시작 내구성 (BR-16)**
  - startTimeUnix가 현재 시각보다 충분히 과거인 activeJob → 즉시 완료 처리 확인

#### 구현
- [ ] `Model/Service/ProductionService.h/.cpp`
  - `checkAndComplete()`: activeJob 로드 → elapsed 계산 → 완료 시 처리
  - `tryCompleteActiveJob()`: 재고 추가 + CONFIRMED 전환 + activeJob 제거 + 파일 저장
  - `startNextJob()`: queue.front() 꺼내기 → startTimeUnix 설정 → activeJob 등록
  - `checkAndComplete()` 재귀 호출 (BR-17)

### 검증
- 모든 테스트 PASS

---

## Phase 3: ProductionLineController + ProductionLineView 구현

### Tasks

#### 테스트 코드
- [ ] `test/ProductionLineControllerTest.cpp` 작성
  - `showStatus()`: activeJob 없을 때 "생산 중 없음" 메시지 출력 확인
  - `showStatus()`: activeJob 있을 때 경과 시간 = `now() - startTimeUnix` / 60 확인
  - `showQueue()`: queue 목록 순서 확인
  - `advanceTime(N)` (--mock-time 모드): `MockTimeProvider::advance(N)` 호출 후 `checkAndComplete()` 트리거 확인

#### 구현
- [ ] `Controller/ProductionLineController.h/.cpp`
  - `showStatus()`: `checkAndComplete()` 호출 → activeJob 조회 → 경과 시간 계산 → View 렌더링
  - `showQueue()`: `getState().queue` → View 렌더링
  - `advanceTime(minutes)` (MockTimeProvider 모드 전용): 시간 앞당기기 + `checkAndComplete()`
- [ ] `View/ProductionLineView.h/.cpp`
  - `showSubMenu(isMockMode)`: [1] 생산 현황 / [2] 대기 큐 / ([3] 시간 앞당기기, mock 모드만) / [0] 돌아가기
  - `showActiveJob(job, sampleName, elapsed, expectedFinish)`: 전체 현황 테이블 출력
  - `showNoActiveJob()`: "현재 생산 중인 작업 없음" 메시지
  - `showQueue(jobs, sampleNames)`: 대기 순번 / 주문 ID / 시료명 / 실생산량 / 예상소요시간 출력
  - `promptAdvanceMinutes()`: 앞당길 분(minutes) 입력

### 검증
- 수동 실행 (`--mock-time` 모드):
  - 주문 접수 → 재고 부족 승인 → 생산 현황 확인 → 시간 앞당기기 → 생산 완료 자동 처리 확인
  - 재고 증가 확인 (시료 관리 목록 조회)
  - CONFIRMED 전환 확인 (모니터링)
  - queue에 여러 항목 → 순차 완료 처리 확인
- 수동 실행: 앱 재시작 후 startTimeUnix 기반 완료 자동 처리 확인
