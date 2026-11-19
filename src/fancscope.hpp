#pragma once
#include "interpreter.hpp"
#include <map>
#include <string>
#include <variant>


struct Data {
    DataType type;
    std::variant<int, bool, double, std::string> data;
};


class FanCScope {

public:
    FanCScope* scopeAbove;
    std::map<std::string, Data> values;

    
};
