#pragma once
#include <ctime>
#include <string>

class ITimeProvider {
public:
    virtual ~ITimeProvider() = default;
    virtual time_t      now() const = 0;
    virtual std::string nowIso8601() const = 0;
    virtual void        advance(int minutes) {}  // mock 모드에서 시간 앞당기기; 실제 구현은 no-op
};
