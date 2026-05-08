#pragma once

class ISubController {
public:
    virtual ~ISubController() = default;
    /// 하위 메뉴 진입점. 사용자가 이 서브시스템을 종료할 때까지 루프를 관리한다.
    virtual void run() = 0;
};
