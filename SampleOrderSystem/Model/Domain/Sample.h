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

    /// 도메인 유효성 검사. 빈 문자열 반환 시 유효, 비어 있지 않으면 오류 메시지.
    std::string validate() const {
        if (yieldRate <= 0.0 || yieldRate > 1.0)
            return "수율은 0 초과 1.0 이하여야 합니다.";
        if (avgProductionTime <= 0)
            return "평균 생산시간은 1분 이상이어야 합니다.";
        return "";
    }
};
