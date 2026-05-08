# Design: 시료 관리 (Sample Management)

> **관련 문서**
> - PRD: [docs/prd/sample-management.md](../prd/sample-management.md)
> - PLAN: [docs/plan/sample-management.md](../plan/sample-management.md)

---

## Phase 1: 기반 인프라 구축

> 모든 기능이 공유하는 인프라. 시료 관리가 첫 번째 구현 기능이므로 여기서 일괄 설정한다.

### Tasks

#### 프로젝트 구조
- [ ] `SampleOrderSystem.vcxproj`에 필터 추가: `Model`, `Model/Domain`, `Model/Repository`, `Model/Service`, `View`, `Controller`, `Tools`, `테스트 파일`
- [ ] `data/` 디렉토리 생성 및 빈 JSON 파일 초기화 (`samples.json`, `orders.json`, `production.json`)

#### json.hpp
- [ ] DataPersistence PoC의 `json.hpp` (`JsonValue` 커스텀 구현)를 `SampleOrderSystem/` 하위에 복사

#### 도메인 구조체
- [ ] `Model/Domain/Sample.h` — `Sample` 구조체 + `toJson()` / `fromJson()` 구현
- [ ] `Model/Domain/Order.h` — `OrderStatus` 열거형 + `Order` 구조체 + 직렬화 구현
- [ ] `Model/Domain/ProductionJob.h` — `ProductionJob` 구조체 + `ProductionState` + 직렬화 구현

#### Repository 인터페이스
- [ ] `Model/Repository/ISampleRepository.h` — `save`, `update`, `findById`, `findAll`, `searchByName`, `existsId`, `existsName`
- [ ] `Model/Repository/IOrderRepository.h` — `create`, `findById`, `findAll`, `findByStatus`, `update`
- [ ] `Model/Repository/IProductionRepository.h` — `getState`, `setState`, `enqueue`

#### ITimeProvider
- [ ] `Model/Service/ITimeProvider.h` — `now()`, `nowIso8601()` 순수 가상 인터페이스
- [ ] `Model/Service/RealTimeProvider.h` — `std::time(nullptr)` 구현
- [ ] `Model/Service/MockTimeProvider.h` — `advance(minutes)`, `setTime(time_t)` 구현

#### MVC 공통 인터페이스
- [ ] `Controller/IController.h` — `run()` 순수 가상 인터페이스
- [ ] `View/IView.h` — `render()` 순수 가상 인터페이스

### 검증
- 빌드 성공 (컴파일 에러 없음)
- `data/` 디렉토리와 빈 JSON 파일 확인

---

## Phase 2: JsonSampleRepository 구현

### Tasks

#### 테스트 코드
- [ ] `test/SampleRepositoryTest.cpp` 작성
  - `save()` 후 `findById()`로 조회 성공
  - 중복 ID: `existsId()` 반환 true
  - 중복 이름: `existsName()` 반환 true
  - `findAll()` 전체 목록 반환
  - `searchByName("GaN")` 부분 일치 결과 반환
  - `update()` 후 재고 수량 변경 확인
  - 앱 재시작 시뮬레이션: 같은 파일로 Repository 재생성 후 데이터 유지 확인

#### 구현
- [ ] `Model/Repository/JsonSampleRepository.h/.cpp`
  - 생성자에서 `data/samples.json` 로드 → 인메모리 `map<string, Sample>` 구성
  - `save()`: upsert + 파일 즉시 flush
  - `update()`: 인메모리 수정 + 파일 flush
  - `findById()`, `findAll()`, `existsId()`, `existsName()`: 인메모리 조회
  - `searchByName()`: `toLower()` 적용 부분 일치 필터

### 검증
- 모든 테스트 PASS
- `data/samples.json` 파일 내용이 PRD 스키마 형식과 일치하는지 육안 확인

---

## Phase 3: SampleController + SampleView 구현

### Tasks

#### 테스트 코드
- [ ] `test/SampleControllerTest.cpp` 작성 (MockSampleRepository 사용)
  - `registerSample()`: 정상 등록 → `save()` 호출 확인
  - `registerSample()`: 중복 ID → 에러 반환, `save()` 미호출 확인
  - `registerSample()`: 중복 이름 → 에러 반환
  - `registerSample()`: 수율 범위 위반(0 이하, 1.0 초과) → 에러 반환
  - `listSamples()`: `findAll()` 결과 View에 전달 확인
  - `searchSamples()`: `searchByName()` 결과 View에 전달 확인

#### 구현
- [ ] `Controller/SampleController.h/.cpp`
  - `registerSample()`: 입력 유효성 검사 → 중복 검사 → `ISampleRepository::save()`
  - `listSamples()`: `findAll()` → View 렌더링
  - `searchSamples()`: 키워드 입력 → `searchByName()` → View 렌더링
- [ ] `View/SampleView.h/.cpp`
  - `showSampleList(samples)`: ID / 이름 / 수율 / 평균생산시간 / 재고 테이블 출력
  - `showSearchResult(samples)`: 검색 결과 테이블 출력
  - `promptSampleInput()`: 필드별 입력 받기
  - `showError(message)`: 에러 메시지 출력
  - `showSuccess(message)`: 성공 메시지 출력
  - `promptKeyword()`: 검색 키워드 입력
  - `showSubMenu()`: [1] 등록 / [2] 목록 / [3] 검색 / [0] 돌아가기

### 검증
- 모든 테스트 PASS
- 수동 실행: 시료 등록 → `data/samples.json` 저장 확인
- 수동 실행: 중복 ID 등록 시 에러 메시지 출력 확인
- 수동 실행: 이름 검색으로 부분 일치 결과 출력 확인
- 수동 실행: 앱 재시작 후 기존 시료 데이터 유지 확인
