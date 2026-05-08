#pragma once
#include "ITimeProvider.h"
#include <ctime>
#include <sstream>
#include <iomanip>

class RealTimeProvider : public ITimeProvider {
public:
    time_t now() const override { return std::time(nullptr); }
    std::string nowIso8601() const override {
        time_t t = now();
        struct tm tm_info;
        localtime_s(&tm_info, &t);
        std::ostringstream ss;
        ss << std::put_time(&tm_info, "%Y-%m-%dT%H:%M:%S");
        return ss.str();
    }
};
