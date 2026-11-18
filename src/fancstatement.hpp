#pragma once

#include "interpreter.hpp"

class FanCStatement {

public:
    virtual void accept(Interpreter interpreter);
    virtual ~FanCStatement() = default;
};
