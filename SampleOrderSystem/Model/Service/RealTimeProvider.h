#pragma once
#include "ITimeProvider.h"
#include "TimeUtils.h"
#include <ctime>

class RealTimeProvider : public ITimeProvider {
public:
    time_t now() const override { return std::time(nullptr); }
    std::string nowIso8601() const override { return TimeUtils::toIso8601(now()); }
};
