#pragma once

class ISubController {
public:
    virtual ~ISubController() = default;
    virtual void run() = 0;
};
