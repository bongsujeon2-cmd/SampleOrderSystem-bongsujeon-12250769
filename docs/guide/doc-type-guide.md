# 문서 유형 가이드

기능 하나를 구현할 때 PRD → PLAN → Design 순서로 문서를 작성한다.  
각 문서는 헤더에 서로를 참조하는 링크를 포함한다.

---

## PRD (Product Requirements Document)

**이 기능을 왜 만드는가**를 설명하는 기획 문서.

- 사용자 스토리
- 비기능 요구사항
- 성공 기준

기술적 결정은 포함하지 않으며, 개발자가 아닌 사람도 읽을 수 있는 수준으로 작성한다.

**경로:** `docs/prd/(feature name).md`

**파일 헤더 예시:**
```markdown
# PRD: (기능명)

> **관련 문서**
> - PLAN: [docs/plan/(feature name).md](../../plan/(feature name).md)
> - Design: [docs/design/(feature name).md](../../design/(feature name).md)
```

---

## PLAN (계획서)

**어떻게 풀 것인가**를 결정하는 기술 설계 문서.

- 기술 방식 비교
- 아키텍처 결정과 근거
- 트레이드오프 분석
- 왜 이 접근을 선택했는지 의사결정 과정

나중에 코드를 읽는 사람이 "왜 이렇게 짰냐"를 이해하는 데 쓰인다.

**경로:** `docs/plan/(feature name).md`

**파일 헤더 예시:**
```markdown
# PLAN: (기능명)

> **관련 문서**
> - PRD: [docs/prd/(feature name).md](../../prd/(feature name).md)
> - Design: [docs/design/(feature name).md](../../design/(feature name).md)
```

---

## Design (설계서)

**실제 구현을 어떻게 할지**에 대한 세부 계획.

- 세부 계획은 Phase 단계로 구분
- Phase별로 Task가 체크리스트 형태로 기입
- 각 Phase마다 검증 방법 기입
- 모든 Phase 구현/검증이 끝나면 기능 구현 완료

**경로:** `docs/design/(feature name).md`

**파일 헤더 예시:**
```markdown
# Design: (기능명)

> **관련 문서**
> - PRD: [docs/prd/(feature name).md](../../prd/(feature name).md)
> - PLAN: [docs/plan/(feature name).md](../../plan/(feature name).md)

## Phase 1: (단계명)

### Tasks
- [ ] Task A
- [ ] Task B

### 검증
- (검증 방법 기술)
```
