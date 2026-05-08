# PLAN: 시료 관리 (Sample Management)

> **관련 문서**
> - PRD: [docs/prd/sample-management.md](../prd/sample-management.md)
> - Design: [docs/design/sample-management.md](../design/sample-management.md)

---

## 1. SampleRepository 설계

### 1.1 왜 JsonRepository\<T\>를 직접 사용하지 않는가

DataPersistence PoC의 `JsonRepository<T>`는 `int64_t` 자동 증가 ID를 가정한다.

```json
// PoC 방식
{ "nextId": 4, "entities": [ { "id": 3, ... } ] }
```

시료 ID는 **사용자가 직접 입력하는 string**(`"S-001"`)이므로:
- auto-increment ID 불필요
- 키 타입이 `string` → `JsonRepository<T>` 계약 불일치

**결정:** `ISampleRepository` 인터페이스 + `JsonSampleRepository` 구현체를 직접 작성한다.
- 인메모리 스토어: `std::map<std::string, Sample> store_`
- 생성자에서 `data/samples.json` 로드
- 쓰기 연산마다 파일 즉시 flush (DataPersistence PoC와 동일 패턴)

### 1.2 ISampleRepository 인터페이스

```cpp
class ISampleRepository {
public:
    virtual ~ISampleRepository() = default;
    virtual bool             save(const Sample& sample) = 0;         // upsert
    virtual bool             update(const Sample& sample) = 0;
    virtual std::optional<Sample> findById(const std::string& id) const = 0;
    virtual std::vector<Sample>   findAll() const = 0;
    virtual std::vector<Sample>   searchByName(const std::string& keyword) const = 0;
    virtual bool             existsId(const std::string& id) const = 0;
    virtual bool             existsName(const std::string& name) const = 0;
};
```

인터페이스 분리로 Google Mock 기반 단위 테스트가 가능해진다.

### 1.3 data/samples.json 파일 형식

PRD 정의 형식 그대로 사용 (PoC의 entities 형식과 다름):

```json
{
  "samples": [
    {
      "id": "S-001",
      "name": "AlGaN 시료",
      "avgProductionTime": 5,
      "yieldRate": 0.9,
      "currentStock": 50
    }
  ]
}
```

---

## 2. 중복 검사 전략

**결정:** 중복 검사는 **Controller 레이어**에서 수행한다.

```
SampleController::registerSample():
  1. ISampleRepository::existsId(id)   → 중복이면 View에 에러 메시지, 종료
  2. ISampleRepository::existsName(name) → 중복이면 View에 에러 메시지, 종료
  3. ISampleRepository::save(sample)
```

- `existsId()`: `map::count()` — O(log n)
- `existsName()`: `findAll()` 후 선형 탐색 — 시료 수가 수십 개 수준이므로 충분
- Repository가 중복을 거부하지 않는 이유: 에러 메시지 출력과 비즈니스 판단은 Controller/View 책임

---

## 3. 검색 구현

- `searchByName(keyword)`: `findAll()` 후 `std::string::find()` 부분 일치 필터링
- 대소문자 구분 없는 검색: DataMonitor PoC의 `toLower()` 헬퍼 패턴 재사용
- 검색은 인메모리에서 수행 → 파일 재파싱 없음

---

## 4. 재고 수량 관리

`currentStock` 필드는 `Sample` 도메인 구조체에 포함된다.

| 변경 주체 | 시점 | 방향 |
|----------|------|------|
| `ShipmentController` | 출고(RELEASE) | 차감 |
| `ProductionService` | 생산 완료 | 증가 |

두 주체 모두 `ISampleRepository::update(sample)`을 통해 변경 후 즉시 파일 저장한다.
재고 조작 경로가 Repository 하나로 집중되어 정합성을 유지한다.

---

## 5. Sample 도메인 구조체

```cpp
struct Sample {
    std::string id;
    std::string name;
    int         avgProductionTime = 0;  // 분(minute)
    double      yieldRate         = 1.0;
    int         currentStock      = 0;

    JsonValue toJson() const;
    static Sample fromJson(const JsonValue& j);
};
```

수율 유효성 검사(`0 < yieldRate <= 1.0`)는 `SampleController`의 입력 처리 단계에서 수행한다.
