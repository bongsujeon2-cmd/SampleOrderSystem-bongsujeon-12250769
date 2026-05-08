#pragma once
#include "../../json.hpp"
#include <string>

struct Sample {
    std::string id;
    std::string name;
    int         avgProductionTime = 0;
    double      yieldRate         = 1.0;
    int         currentStock      = 0;

    JsonValue toJson() const {
        JsonValue j;
        j["id"]                = JsonValue(id);
        j["name"]              = JsonValue(name);
        j["avgProductionTime"] = JsonValue(avgProductionTime);
        j["yieldRate"]         = JsonValue(yieldRate);
        j["currentStock"]      = JsonValue(currentStock);
        return j;
    }

    static Sample fromJson(const JsonValue& j) {
        Sample s;
        s.id                = j.at("id").getString();
        s.name              = j.at("name").getString();
        s.avgProductionTime = static_cast<int>(j.at("avgProductionTime").getInt());
        s.yieldRate         = j.at("yieldRate").getNumber();
        s.currentStock      = static_cast<int>(j.at("currentStock").getInt());
        return s;
    }
};
