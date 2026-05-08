#pragma once
#include "ISampleRepository.h"
#include "../../json.hpp"
#include <map>
#include <string>
#include <algorithm>
#include <cctype>

class JsonSampleRepository : public ISampleRepository {
public:
    explicit JsonSampleRepository(const std::string& filePath)
        : filePath_(filePath)
    {
        load();
    }

    // insert (중복 ID는 false 반환). Controller가 existsId()로 사전 검사 후 호출하는 계약.
    bool save(const Sample& sample) override {
        if (store_.count(sample.id)) return false;
        store_[sample.id] = sample;
        return flush();
    }

    bool update(const Sample& sample) override {
        if (!store_.count(sample.id)) return false;
        store_[sample.id] = sample;
        return flush();
    }

    std::optional<Sample> findById(const std::string& id) const override {
        auto it = store_.find(id);
        if (it == store_.end()) return std::nullopt;
        return it->second;
    }

    std::vector<Sample> findAll() const override {
        std::vector<Sample> result;
        result.reserve(store_.size());
        for (const auto& [id, sample] : store_)
            result.push_back(sample);
        return result;
    }

    std::vector<Sample> searchByName(const std::string& keyword) const override {
        std::string lowerKeyword = toLower(keyword);
        std::vector<Sample> result;
        for (const auto& [id, sample] : store_) {
            if (toLower(sample.name).find(lowerKeyword) != std::string::npos)
                result.push_back(sample);
        }
        return result;
    }

    bool existsId(const std::string& id) const override {
        return store_.count(id) > 0;
    }

    // 대소문자를 구분하는 완전 일치. "AlGaN"과 "algan"은 다른 이름으로 취급한다.
    bool existsName(const std::string& name) const override {
        for (const auto& [id, sample] : store_) {
            if (sample.name == name) return true;
        }
        return false;
    }

private:
    std::string                   filePath_;
    std::map<std::string, Sample> store_;

    void load() {
        JsonValue root = JsonValue::parseFile(filePath_);
        if (!root.isObject()) return;
        if (!root.contains("samples")) return;
        const auto& arr = root.at("samples");
        if (!arr.isArray()) return;
        for (size_t i = 0; i < arr.size(); ++i) {
            Sample s = sampleFromJson(arr[i]);
            store_[s.id] = s;
        }
    }

    bool flush() const {
        JsonValue root;
        JsonValue arr = JsonValue(JsonValue::Array{});
        for (const auto& [id, sample] : store_)
            arr.getArray().push_back(sampleToJson(sample));
        root["samples"] = arr;
        return root.saveToFile(filePath_);
    }

    static JsonValue sampleToJson(const Sample& s) {
        JsonValue j;
        j["id"]                = JsonValue(s.id);
        j["name"]              = JsonValue(s.name);
        j["avgProductionTime"] = JsonValue(s.avgProductionTime);
        j["yieldRate"]         = JsonValue(s.yieldRate);
        j["currentStock"]      = JsonValue(s.currentStock);
        return j;
    }

    static Sample sampleFromJson(const JsonValue& j) {
        Sample s;
        s.id                = j.at("id").getString();
        s.name              = j.at("name").getString();
        s.avgProductionTime = static_cast<int>(j.at("avgProductionTime").getInt());
        s.yieldRate         = j.at("yieldRate").getNumber();
        s.currentStock      = static_cast<int>(j.at("currentStock").getInt());
        return s;
    }

    static std::string toLower(const std::string& s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return result;
    }
};
