#pragma once
#include "ITimeProvider.h"
#include "TimeUtils.h"
#include <ctime>

class MockTimeProvider : public ITimeProvider {
public:
    MockTimeProvider() : current_(std::time(nullptr)) {}
    explicit MockTimeProvider(time_t t) : current_(t) {}
    time_t now() const override { return current_; }
    void advance(int minutes) { current_ += static_cast<time_t>(minutes) * 60; }
    void setTime(time_t t) { current_ = t; }
    std::string nowIso8601() const override { return TimeUtils::toIso8601(current_); }
private:
    time_t current_;
};
