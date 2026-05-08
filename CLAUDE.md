# CLAUDE.md
프로젝트는 꼭 아래 내용을 참고하여 작업을 진행해야한다.

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
```

## 프로젝트 구조

- `SampleOrderSystem.slnx` — Visual Studio 솔루션 (`.slnx` 형식, VS 2022 이상)
- `SampleOrderSystem/SampleOrderSystem.vcxproj` — 메인 C++ 프로젝트
- `packages/gmock.1.11.0/` — Google Mock 1.11.0 (NuGet, 이미 복원됨)

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

## 커밋 규칙

커밋 시 반드시 [docs/guide/commit-guide.md](docs/guide/commit-guide.md)를 따른다.

## 기획/설계/구현 문서 작성 가이드

기능 구현 시 **PRD → PLAN → Design** 순서로 문서를 작성한다.

- [docs/guide/doc-type-guide.md](docs/guide/doc-type-guide.md) — 각 문서 유형의 목적, 작성 기준, 파일 경로 규칙, 헤더 예시
