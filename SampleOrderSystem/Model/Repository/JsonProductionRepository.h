#pragma once
#include "IProductionRepository.h"
#include "../../json.hpp"
#include <string>
#include <deque>
#include <stdexcept>

class JsonProductionRepository : public IProductionRepository {
public:
    explicit JsonProductionRepository(const std::string& filePath)
        : filePath_(filePath)
    {
        load();
    }

    ProductionState getState() const override {
        return state_;
    }

    void setState(const ProductionState& state) override {
        state_ = state;
        if (!flush()) {
            throw std::runtime_error("프로덕션 상태 저장 실패");
        }
    }

    void enqueue(const ProductionJob& job) override {
        state_.queue.push_back(job);
        if (!flush()) {
            throw std::runtime_error("프로덕션 상태 저장 실패");
        }
    }

    // Repository 스토어를 완전히 초기화하고 빈 파일로 저장.
    // SemiDummyGenerator의 run(false=덮어쓰기) 시나리오에서 사용.
    void clearAll() override {
        state_.activeJob = std::nullopt;
        state_.queue.clear();
        flush();
    }

private:
    std::string     filePath_;
    ProductionState state_;

    void load() {
        JsonValue root = JsonValue::parseFile(filePath_);
        if (!root.isObject()) return;

        // activeJob
        if (root.contains("activeJob") && !root.at("activeJob").isNull()) {
            state_.activeJob = jobFromJson(root.at("activeJob"));
        } else {
            state_.activeJob = std::nullopt;
        }

        // queue
        state_.queue.clear();
        if (root.contains("queue") && root.at("queue").isArray()) {
            const auto& arr = root.at("queue");
            for (size_t i = 0; i < arr.size(); ++i) {
                state_.queue.push_back(jobFromJson(arr[i]));
            }
        }
    }

    bool flush() const {
        JsonValue root;

        // activeJob
        if (state_.activeJob.has_value()) {
            root["activeJob"] = jobToJson(state_.activeJob.value());
        } else {
            root["activeJob"] = JsonValue(nullptr);
        }

        // queue
        JsonValue arr = JsonValue(JsonValue::Array{});
        for (const auto& job : state_.queue) {
            arr.getArray().push_back(jobToJson(job));
        }
        root["queue"] = arr;

        return root.saveToFile(filePath_);
    }

    static JsonValue jobToJson(const ProductionJob& job) {
        JsonValue j;
        j["orderId"]                = JsonValue(job.orderId);
        j["sampleId"]               = JsonValue(job.sampleId);
        j["shortage"]               = JsonValue(job.shortage);
        j["actualProductionQty"]    = JsonValue(job.actualProductionQty);
        j["totalProductionTimeMin"] = JsonValue(job.totalProductionTimeMin);
        j["startTimeUnix"]          = JsonValue(job.startTimeUnix);
        return j;
    }

    static ProductionJob jobFromJson(const JsonValue& j) {
        ProductionJob job;
        job.orderId                = j.at("orderId").getString();
        job.sampleId               = j.at("sampleId").getString();
        job.shortage               = static_cast<int>(j.at("shortage").getInt());
        job.actualProductionQty    = static_cast<int>(j.at("actualProductionQty").getInt());
        job.totalProductionTimeMin = static_cast<int>(j.at("totalProductionTimeMin").getInt());
        job.startTimeUnix          = j.at("startTimeUnix").getInt();
        return job;
    }
};
