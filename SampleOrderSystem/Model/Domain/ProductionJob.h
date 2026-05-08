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
            && totalProductionTimeMin == o.totalProductionTimeMin;
    }
};

struct ProductionState {
    std::optional<ProductionJob> activeJob;
    std::deque<ProductionJob>    queue;
};
