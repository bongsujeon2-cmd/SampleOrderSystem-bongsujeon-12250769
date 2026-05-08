#pragma once
#include "IOrderRepository.h"
#include "../../json.hpp"
#include <map>
#include <string>
#include <iomanip>
#include <sstream>
#include <stdexcept>

class JsonOrderRepository : public IOrderRepository {
public:
    explicit JsonOrderRepository(const std::string& filePath)
        : filePath_(filePath), nextOrdNum_(1)
    {
        load();
    }

    std::string create(const Order& order) override {
        Order o = order;
        o.id = generateId();
        store_[o.id] = o;
        if (!flush())
            throw std::runtime_error("파일 저장 실패");
        return o.id;
    }

    std::optional<Order> findById(const std::string& id) const override {
        auto it = store_.find(id);
        if (it == store_.end()) return std::nullopt;
        return it->second;
    }

    std::vector<Order> findAll() const override {
        std::vector<Order> result;
        result.reserve(store_.size());
        for (const auto& [id, order] : store_)
            result.push_back(order);
        return result;
    }

    std::vector<Order> findByStatus(OrderStatus status) const override {
        std::vector<Order> result;
        for (const auto& [id, order] : store_) {
            if (order.status == status)
                result.push_back(order);
        }
        return result;
    }

    bool update(const Order& order) override {
        if (!store_.count(order.id)) return false;
        store_[order.id] = order;
        flush(); // flush() const: 설계상 실패 시 로그 처리 또는 상위 레이어에서 처리
        return true;
    }

    // Repository 스토어를 완전히 초기화하고 빈 파일로 저장.
    // SemiDummyGenerator의 run(false=덮어쓰기) 시나리오에서 사용.
    void clearAll() override {
        store_.clear();
        nextOrdNum_ = 1;
        flush();
    }

private:
    std::string                  filePath_;
    std::map<std::string, Order> store_;
    int                          nextOrdNum_;

    std::string generateId() {
        std::ostringstream ss;
        ss << "ORD-" << std::setw(3) << std::setfill('0') << nextOrdNum_++;
        return ss.str();
    }

    void load() {
        JsonValue root = JsonValue::parseFile(filePath_);
        if (!root.isObject()) return;

        if (root.contains("nextOrdNum"))
            nextOrdNum_ = static_cast<int>(root.at("nextOrdNum").getInt());

        if (nextOrdNum_ < 1) nextOrdNum_ = 1;

        if (!root.contains("orders")) return;
        const auto& arr = root.at("orders");
        if (!arr.isArray()) return;
        for (size_t i = 0; i < arr.size(); ++i) {
            Order o = orderFromJson(arr[i]);
            store_[o.id] = o;
        }
    }

    // flush()는 non-const 멤버를 변경하지 않으나, 설계상 mutable화 없이 const로 유지
    bool flush() const {
        JsonValue root;
        root["nextOrdNum"] = JsonValue(nextOrdNum_);
        JsonValue arr = JsonValue(JsonValue::Array{});
        for (const auto& [id, order] : store_)
            arr.getArray().push_back(orderToJson(order));
        root["orders"] = arr;
        return root.saveToFile(filePath_);
    }

    static std::string statusToString(OrderStatus status) {
        switch (status) {
        case OrderStatus::RESERVED:  return "RESERVED";
        case OrderStatus::PRODUCING: return "PRODUCING";
        case OrderStatus::CONFIRMED: return "CONFIRMED";
        case OrderStatus::RELEASE:   return "RELEASE";
        case OrderStatus::REJECTED:  return "REJECTED";
        default: throw std::invalid_argument("Unknown OrderStatus");
        }
    }

    static OrderStatus statusFromString(const std::string& s) {
        if (s == "RESERVED")  return OrderStatus::RESERVED;
        if (s == "PRODUCING") return OrderStatus::PRODUCING;
        if (s == "CONFIRMED") return OrderStatus::CONFIRMED;
        if (s == "RELEASE")   return OrderStatus::RELEASE;
        if (s == "REJECTED")  return OrderStatus::REJECTED;
        throw std::invalid_argument("Unknown OrderStatus: " + s);
    }

    static JsonValue orderToJson(const Order& o) {
        JsonValue j;
        j["id"]           = JsonValue(o.id);
        j["sampleId"]     = JsonValue(o.sampleId);
        j["customerName"] = JsonValue(o.customerName);
        j["quantity"]     = JsonValue(o.quantity);
        j["status"]       = JsonValue(statusToString(o.status));
        j["createdAt"]    = JsonValue(o.createdAt);
        return j;
    }

    static Order orderFromJson(const JsonValue& j) {
        Order o;
        o.id           = j.at("id").getString();
        o.sampleId     = j.at("sampleId").getString();
        o.customerName = j.at("customerName").getString();
        o.quantity     = static_cast<int>(j.at("quantity").getInt());
        o.status       = statusFromString(j.at("status").getString());
        o.createdAt    = j.at("createdAt").getString();
        return o;
    }
};
