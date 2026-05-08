# CLAUDE.md
프로젝트는 꼭 아래 내용을 참고하여 작업을 진행해야한다.

## 프로젝트 개요

**반도체 시료 생산주문관리 시스템 (S-Semi)**
가상의 반도체 회사 S-Semi가 연구소·팹리스·대학 연구실에 반도체 시료를 생산·납품하는 콘솔 애플리케이션.

- **아키텍처:** MVC 패턴
- **데이터 저장:** JSON 파일 기반 영속성

## 빌드

Visual Studio 2022 (MSVC v145) C++20 Windows 콘솔 애플리케이션.

**MSBuild 명령어:**
```
msbuild SampleOrderSystem.slnx /p:Configuration=Debug /p:Platform=x64
msbuild SampleOrderSystem.slnx /p:Configuration=Release /p:Platform=x64
```

**테스트 실행:**
```
.\x64\Debug\SampleOrderSystem.exe
.\x64\Debug\SampleOrderSystem.exe --mock-time   # 시간 Mocking 모드
```

## 프로젝트 구조

- `SampleOrderSystem.slnx` — Visual Studio 솔루션 (`.slnx` 형식, VS 2022 이상)
- `SampleOrderSystem/SampleOrderSystem.vcxproj` — 메인 C++ 프로젝트
- `packages/gmock.1.11.0/` — Google Mock 1.11.0 (NuGet, 이미 복원됨)
- 
## 주요 설정

- **C++ 표준:** C++20 (`/std:c++20`)
- **문자 집합:** 유니코드
- **빌드 구성:** Debug/Release × Win32/x64
- **테스트 프레임워크:** Google Mock 1.11.0 (gmock) — gtest 헤더 포함

## 테스트

Google Mock은 NuGet(`packages/gmock.1.11.0`)으로 참조된다. 테스트 파일에 아래 헤더를 포함한다:
```cpp
#include <gmock/gmock.h>
#include <gtest/gtest.h>
```
테스트 소스 파일을 프로젝트에 추가하면 gmock 타겟과 자동으로 링크된다 (`.vcxproj`의 `gmock.targets` import로 설정됨).

## 기능 개발 절차 (필수 준수)

기능 하나를 구현할 때 아래 4단계를 **반드시 순서대로** 거쳐야 한다.

```
[1단계: 검증] → [2단계: 구현] → [3단계: 리뷰] → [4단계: 리팩토링 + 재검증]
```

### 1단계 — 테스트 코드 작성 (검증 전문 subagent)

- **검증 전문 subagent**가 구현보다 먼저 테스트 코드를 작성한다 (TDD).
- 테스트 파일 위치: `test/` 폴더 하위.
- 작성 기준: 해당 기능의 PRD 요구사항과 비즈니스 규칙(BR)을 모두 커버.
- 이 시점에서는 구현이 없으므로 테스트는 **FAIL** 상태여야 정상.

### 2단계 — 구현 (구현 전문 subagent)

- **구현 전문 subagent**가 테스트를 PASS시키기 위한 최소한의 구현을 작성한다.
- 테스트가 모두 **PASS**되면 커밋을 진행한다.
- 커밋 규칙: [docs/guide/commit-guide.md](docs/guide/commit-guide.md) 준수.

### 3단계 — 코드 리뷰 (리뷰 전문 subagent)

- **리뷰 전문 subagent**가 구현 코드를 아래 원칙에 따라 검토한다.
  - **SRP** (단일 책임 원칙)
  - **SOLID** 원칙 전체
  - **Clean Code** (가독성, 네이밍, 중복 제거 등)
- 리뷰 결과에서 수정이 필요한 항목은 **구현 전문 subagent**에 리팩토링을 요청한다.

### 4단계 — 리팩토링 + 재검증 (구현 subagent → 검증 subagent)

- **구현 전문 subagent**가 리뷰 피드백을 반영하여 리팩토링을 수행한다.
- 리팩토링 완료 후 **검증 전문 subagent**가 전체 테스트를 재실행한다.
- 모든 테스트가 **PASS**되면 커밋을 진행한다.

---

## 커밋 규칙

커밋 시 반드시 [docs/guide/commit-guide.md](docs/guide/commit-guide.md)를 따른다.
** Push는 꼭 사용자 허락을 받고 진행한다 **

## 기획/설계/구현 문서 작성 가이드

기능 구현 시 **PRD → PLAN → Design** 순서로 문서를 작성한다.

- [docs/master_prd.md](docs/master_prd.md) — 마스터 PRD (전체 요구사항 원문)
- [docs/guide/doc-type-guide.md](docs/guide/doc-type-guide.md) — 각 문서 유형의 목적, 작성 기준, 파일 경로 규칙, 헤더 예시
