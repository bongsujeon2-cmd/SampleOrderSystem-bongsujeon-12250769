#pragma once
#include <string>

struct Sample {
    std::string id;
    std::string name;
    int         avgProductionTime = 0;
    double      yieldRate         = 1.0;
    int         currentStock      = 0;

    bool operator==(const Sample& other) const {
        return id                == other.id
            && name              == other.name
            && avgProductionTime == other.avgProductionTime
            && yieldRate         == other.yieldRate
            && currentStock      == other.currentStock;
    }

    bool operator!=(const Sample& other) const {
        return !(*this == other);
    }
};
