#pragma once
#include <string>

struct Sample {
    std::string id;
    std::string name;
    int         avgProductionTime = 0;
    double      yieldRate         = 1.0;
    int         currentStock      = 0;
};
