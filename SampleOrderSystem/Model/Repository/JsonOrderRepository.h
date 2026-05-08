#pragma once
#include "IOrderRepository.h"
#include "../../json.hpp"
#include <map>
#include <string>
#include <iomanip>
#include <sstream>

class JsonOrderRepository : public IOrderRepository {
public:
    explicit JsonOrderRepository(const std::string& filePath)
        : filePath_(filePath), nextOrdNum_(1)
    {
        load();
    }

    std::string create(Order order) override {
        std::ostringstream oss;
        oss << "ORD-" << std::setw(3) << std::setfill('0') << nextOrdNum_++;
        order.id = oss.str();
        store_[order.id] = order;
        flush();
        return order.id;
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
        flush();
        return true;
    }

private:
    std::string              filePath_;
    std::map<std::string, Order> store_;
    int                      nextOrdNum_;

    void load() {
        JsonValue root = JsonValue::parseFile(filePath_);
        if (!root.isObject()) return;

        if (root.contains("nextOrdNum"))
            nextOrdNum_ = static_cast<int>(root.at("nextOrdNum").getInt());

        if (!root.contains("orders")) return;
        const auto& arr = root.at("orders");
        if (!arr.isArray()) return;
        for (size_t i = 0; i < arr.size(); ++i) {
            Order o = orderFromJson(arr[i]);
            store_[o.id] = o;
        }
    }

    bool flush() const {
        JsonValue root;
        root["nextOrdNum"] = JsonValue(nextOrdNum_);
        JsonValue arr = JsonValue(JsonValue::Array{});
        for (const auto& [id, order] : store_)
            arr.getArray().push_back(orderToJson(order));
        root["orders"] = arr;
        return root.saveToFile(filePath_);
    }

    static JsonValue orderToJson(const Order& o) {
        JsonValue j;
        j["id"]           = JsonValue(o.id);
        j["sampleId"]     = JsonValue(o.sampleId);
        j["customerName"] = JsonValue(o.customerName);
        j["quantity"]     = JsonValue(o.quantity);
        j["status"]       = JsonValue(toString(o.status));
        j["createdAt"]    = JsonValue(o.createdAt);
        return j;
    }

    static Order orderFromJson(const JsonValue& j) {
        Order o;
        o.id           = j.at("id").getString();
        o.sampleId     = j.at("sampleId").getString();
        o.customerName = j.at("customerName").getString();
        o.quantity     = static_cast<int>(j.at("quantity").getInt());
        o.status       = orderStatusFromString(j.at("status").getString());
        o.createdAt    = j.at("createdAt").getString();
        return o;
    }
};
