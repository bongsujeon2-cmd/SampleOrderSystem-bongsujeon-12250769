#pragma once
// BR 준수 테스트 데이터 생성기.
// DummyDataGenerator PoC의 랜덤 유틸리티 패턴을 참조하되,
// S-Semi 도메인 비즈니스 규칙(BR-01~BR-18)을 명시적으로 보장하는 고정 시나리오 사용.
#include "../Model/Repository/ISampleRepository.h"
#include "../Model/Repository/IOrderRepository.h"
#include "../Model/Repository/IProductionRepository.h"
#include <random>

class SemiDummyGenerator {
public:
    SemiDummyGenerator(ISampleRepository&, IOrderRepository&, IProductionRepository&);
    int run(bool append = false);

private:
    ISampleRepository&     sampleRepo_;
    IOrderRepository&      orderRepo_;
    IProductionRepository& productionRepo_;
    std::mt19937           rng_;

    std::vector<Sample>    generateSamples();
    std::vector<Order>     generateOrders(const std::vector<Sample>& samples);
    void generateProduction(const std::vector<Order>& orders,
                            const std::vector<Sample>& samples);
    ProductionJob buildJob(const Order& o, const Sample& sample) const;

    template<typename T>
    T randInt(T lo, T hi) {
        std::uniform_int_distribution<T> d(lo, hi);
        return d(rng_);
    }
    double randDouble(double lo, double hi) {
        std::uniform_real_distribution<double> d(lo, hi);
        return d(rng_);
    }
    template<typename C>
    const typename C::value_type& pick(const C& c) {
        return c[randInt<size_t>(0, c.size() - 1)];
    }
};
