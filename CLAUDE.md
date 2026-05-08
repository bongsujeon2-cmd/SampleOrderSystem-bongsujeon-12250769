# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

Visual Studio 2022 (MSVC v145) C++20 Windows console application.

**Build via MSBuild (command line):**
```
msbuild SampleOrderSystem.slnx /p:Configuration=Debug /p:Platform=x64
msbuild SampleOrderSystem.slnx /p:Configuration=Release /p:Platform=x64
```

**Run tests:**
```
.\x64\Debug\SampleOrderSystem.exe
```

## Project Structure

- `SampleOrderSystem.slnx` — Visual Studio solution (new `.slnx` format, VS 2022+)
- `SampleOrderSystem/SampleOrderSystem.vcxproj` — Main C++ project
- `packages/gmock.1.11.0/` — Google Mock 1.11.0 (NuGet, already restored)

## Key Configuration

- **C++ Standard:** C++20 (`/std:c++20`)
- **Character Set:** Unicode
- **Configurations:** Debug/Release × Win32/x64
- **Test Framework:** Google Mock 1.11.0 (gmock) — includes gtest headers

## Testing

Google Mock is referenced via NuGet (`packages/gmock.1.11.0`). Test files should include:
```cpp
#include <gmock/gmock.h>
#include <gtest/gtest.h>
```

Add test source files to the project and link against the gmock targets (already configured via `gmock.targets` import in `.vcxproj`).
