#pragma once
#include "../Domain/Sample.h"
#include <optional>
#include <vector>
#include <string>

class ISampleRepository {
public:
    virtual ~ISampleRepository() = default;
    virtual bool                  save(const Sample& sample) = 0;
    virtual bool                  update(const Sample& sample) = 0;
    virtual std::optional<Sample> findById(const std::string& id) const = 0;
    virtual std::vector<Sample>   findAll() const = 0;
    virtual std::vector<Sample>   searchByName(const std::string& keyword) const = 0;
    virtual bool                  existsId(const std::string& id) const = 0;
    virtual bool                  existsName(const std::string& name) const = 0;
    virtual void                  clearAll() = 0;
};
