#pragma once
#include "ITimeProvider.h"
#include <ctime>
#include <sstream>
#include <iomanip>

class MockTimeProvider : public ITimeProvider {
public:
    MockTimeProvider() : current_(std::time(nullptr)) {}
    explicit MockTimeProvider(time_t t) : current_(t) {}
    time_t now() const override { return current_; }
    void advance(int minutes) override { current_ += static_cast<time_t>(minutes) * 60; }
    void setTime(time_t t) { current_ = t; }
    std::string nowIso8601() const override {
        time_t t = current_;
        struct tm tm_info;
        localtime_s(&tm_info, &t);
        std::ostringstream ss;
        ss << std::put_time(&tm_info, "%Y-%m-%dT%H:%M:%S");
        return ss.str();
    }
private:
    time_t current_;
};
