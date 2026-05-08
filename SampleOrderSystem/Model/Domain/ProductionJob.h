#pragma once
#include <string>
#include <optional>
#include <deque>
#include <cstdint>

struct ProductionJob {
    std::string orderId;
    std::string sampleId;
    int         shortage               = 0;
    int         actualProductionQty    = 0;
    int         totalProductionTimeMin = 0;
    int64_t     startTimeUnix          = 0;

    bool operator==(const ProductionJob& o) const {
        return orderId == o.orderId && sampleId == o.sampleId
            && shortage == o.shortage
            && actualProductionQty == o.actualProductionQty
            && totalProductionTimeMin == o.totalProductionTimeMin
            && startTimeUnix == o.startTimeUnix;
    }
};

// ProductionState는 Repository 계층에서 사용하는 집계 구조체.
// 도메인 레이어(ProductionJob)와 함께 배치하되, 영속성 레이어의 관심사임을 명시한다.
struct ProductionState {
    std::optional<ProductionJob> activeJob;
    std::deque<ProductionJob>    queue;
};
