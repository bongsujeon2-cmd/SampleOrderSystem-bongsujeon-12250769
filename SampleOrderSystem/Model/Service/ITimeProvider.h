#pragma once
#include <ctime>
#include <string>

class ITimeProvider {
public:
    virtual ~ITimeProvider() = default;
    virtual time_t      now() const = 0;
    virtual std::string nowIso8601() const = 0;
};
