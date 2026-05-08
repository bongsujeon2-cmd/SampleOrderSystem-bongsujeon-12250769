// SemiDummyGenerator.cpp
// BR-01~BR-18 비즈니스 규칙을 준수하는 시나리오 기반 더미 데이터 생성기 구현.

#include "SemiDummyGenerator.h"
#include "../Model/Service/TimeUtils.h"

#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <ctime>
#include <cassert>
#include <algorithm>
#include <unordered_map>

// -----------------------------------------------------------------------
// 상수
// -----------------------------------------------------------------------

static const std::vector<std::string> kSampleNames = {
    "AlGaN", "GaN", "SiC", "InP", "GaAs",
    "InGaAs", "AlN", "Si", "Ge", "ZnO"
};

static const std::vector<std::string> kCustomerNames = {
    "Seoul Fab", "Busan Lab", "Daejeon Research", "KAIST", "POSTECH",
    "Korea Univ", "Hanyang Univ", "Sungkyunkwan Univ", "ETRI", "KIST"
};

// -----------------------------------------------------------------------
// 헬퍼: 3자리 0패딩 ID 생성
// -----------------------------------------------------------------------

static std::string makePaddedId(const std::string& prefix, int num) {
    std::ostringstream ss;
    ss << prefix << std::setw(3) << std::setfill('0') << num;
    return ss.str();
}

// -----------------------------------------------------------------------
// 생성자
// -----------------------------------------------------------------------

SemiDummyGenerator::SemiDummyGenerator(
    ISampleRepository&     sampleRepo,
    IOrderRepository&      orderRepo,
    IProductionRepository& productionRepo)
    : sampleRepo_(sampleRepo)
    , orderRepo_(orderRepo)
    , productionRepo_(productionRepo)
    , rng_(std::mt19937(42))   // 고정 시드: 결정론적 시나리오
{
}

// -----------------------------------------------------------------------
// generateSamples() — 5~10개 시료 생성
// -----------------------------------------------------------------------

std::vector<Sample> SemiDummyGenerator::generateSamples() {
    std::vector<Sample> samples;
    int count = randInt(5, 10);

    for (int i = 0; i < count; ++i) {
        Sample s;
        s.id   = makePaddedId("S-", i + 1);
        s.name = kSampleNames[i % kSampleNames.size()];
        // avgProductionTime: 3~20분 (BR: >= 1)
        s.avgProductionTime = randInt(3, 20);
        // yieldRate: 0.70~1.00, 소수점 2자리 (BR-14: 0 < yieldRate <= 1.0)
        s.yieldRate = std::round(randDouble(0.70, 1.00) * 100.0) / 100.0;
        s.currentStock = randInt(10, 100);

        sampleRepo_.save(s);
        samples.push_back(s);
    }
    return samples;
}

// -----------------------------------------------------------------------
// generateOrders() — 10~20개 주문 생성
// -----------------------------------------------------------------------

std::vector<Order> SemiDummyGenerator::generateOrders(const std::vector<Sample>& samples) {
    std::vector<Order> orders;
    int count = randInt(10, 20);

    for (int i = 0; i < count; ++i) {
        const Sample& sample = pick(samples);

        Order o;
        o.sampleId    = sample.id;
        o.customerName = kCustomerNames[i % kCustomerNames.size()];
        o.createdAt   = TimeUtils::toIso8601(std::time(nullptr));

        // 상태 비율: i%10 < 2 → RESERVED, i%10 < 5 → PRODUCING, 나머지 → CONFIRMED
        if (i % 10 < 2) {
            o.status   = OrderStatus::RESERVED;
            o.quantity = randInt(1, 30);
        } else if (i % 10 < 5) {
            // PRODUCING: quantity > currentStock (BR-04)
            int stock    = sample.currentStock;
            o.quantity   = stock + randInt(1, 20);
            o.status     = OrderStatus::PRODUCING;
        } else {
            // CONFIRMED: quantity <= currentStock (재고 충족)
            int maxQty = std::max(1, sample.currentStock / 2);
            o.quantity  = randInt(1, maxQty);
            o.status    = OrderStatus::CONFIRMED;
        }

        std::string assignedId = orderRepo_.create(o);
        o.id = assignedId;
        orders.push_back(o);
    }
    return orders;
}

// -----------------------------------------------------------------------
// buildJob() — ProductionJob 생성 헬퍼 (BR-06, BR-07)
// -----------------------------------------------------------------------

ProductionJob SemiDummyGenerator::buildJob(const Order& o, const Sample& sample) const {
    int shortage = o.quantity - sample.currentStock;
    assert(shortage >= 1 && "PRODUCING 주문의 shortage >= 1 이어야 합니다 (BR-05)");

    // BR-06: actualProductionQty = ceil(shortage / (yieldRate * 0.9))
    // epsilon guard(1e-9)로 부동소수점 오차 방지
    int actualQty = static_cast<int>(
        std::ceil(static_cast<double>(shortage) / (sample.yieldRate * 0.9 - 1e-9))
    );
    // BR-07: totalProductionTimeMin = avgProductionTime * actualQty
    ProductionJob job;
    job.orderId                = o.id;
    job.sampleId               = o.sampleId;
    job.shortage               = shortage;
    job.actualProductionQty    = actualQty;
    job.totalProductionTimeMin = sample.avgProductionTime * actualQty;
    job.startTimeUnix          = 0;
    return job;
}

// -----------------------------------------------------------------------
// generateProduction() — PRODUCING 주문을 activeJob/queue에 연동
// -----------------------------------------------------------------------

void SemiDummyGenerator::generateProduction(
    const std::vector<Order>& orders,
    const std::vector<Sample>& samples)
{
    // 시료 ID → 시료 매핑
    std::unordered_map<std::string, Sample> sampleMap;
    for (const auto& s : samples)
        sampleMap[s.id] = s;

    // PRODUCING 주문 수집 (ID 오름차순 = 생성 순서, FIFO 보장 BR-08)
    std::vector<Order> producingOrders;
    for (const auto& o : orders) {
        if (o.status == OrderStatus::PRODUCING)
            producingOrders.push_back(o);
    }

    // ID 기준 정렬 (ORD-001 < ORD-002 ...)
    std::sort(producingOrders.begin(), producingOrders.end(),
        [](const Order& a, const Order& b) { return a.id < b.id; });

    ProductionState state;

    if (producingOrders.empty()) {
        // PRODUCING 주문 없음 → 빈 상태 저장
        productionRepo_.setState(state);
        return;
    }

    // 현재 시간 (Unix)
    std::time_t now = std::time(nullptr);

    // producingOrders[0] → activeJob
    {
        const Order& o = producingOrders[0];
        const Sample& sample = sampleMap.at(o.sampleId);

        ProductionJob job = buildJob(o, sample);
        // startTimeUnix: 현재 - (totalMin/2 * 60) → 아직 미완료
        // 참고: totalMin/2 정수 절사로 실제 경과 시간이 약간 짧게 설정되어 미완료 보장에 충분한 여유가 있음
        job.startTimeUnix = static_cast<int64_t>(now) - static_cast<int64_t>(job.totalProductionTimeMin / 2) * 60;

        state.activeJob = job;
    }

    // producingOrders[1..] → queue
    for (size_t idx = 1; idx < producingOrders.size(); ++idx) {
        const Order& o = producingOrders[idx];
        const Sample& sample = sampleMap.at(o.sampleId);

        state.queue.push_back(buildJob(o, sample));
    }

    productionRepo_.setState(state);
}

// -----------------------------------------------------------------------
// run()
// -----------------------------------------------------------------------

int SemiDummyGenerator::run(bool append) {
    if (!append) {
        // 덮어쓰기 모드: 기존 데이터 초기화 (DIP 준수 — 인터페이스 메서드 직접 호출)
        sampleRepo_.clearAll();
        orderRepo_.clearAll();
        productionRepo_.clearAll();
    }

    std::cout << "[SemiDummyGenerator] 더미 데이터 생성 시작...\n";

    auto samples    = generateSamples();
    auto orders     = generateOrders(samples);
    generateProduction(orders, samples);

    std::cout << "[SemiDummyGenerator] 완료: 시료 " << samples.size()
              << "개, 주문 " << orders.size() << "개 생성.\n";

    return 0;
}
