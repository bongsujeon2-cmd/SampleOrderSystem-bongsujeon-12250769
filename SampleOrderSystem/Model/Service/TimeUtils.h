#pragma once
#include <ctime>
#include <string>
#include <sstream>
#include <iomanip>

namespace TimeUtils {
    inline std::string toIso8601(time_t t) {
        struct tm tm_info;
        localtime_s(&tm_info, &t);
        std::ostringstream ss;
        ss << std::put_time(&tm_info, "%Y-%m-%dT%H:%M:%S");
        return ss.str();
    }
}
