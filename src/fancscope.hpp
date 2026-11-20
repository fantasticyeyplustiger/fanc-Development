#pragma once
#include "fancerror.hpp"
#include "datatype.hpp"
#include "token.hpp"
#include <map>
#include <string>
#include <variant>
#include <vector>


struct Data {
    DataType type;
    std::variant<int, bool, double, std::string> data;

    Data(DataType type, std::variant<int, bool, double, std::string> data) : type(type), data(data) {};
    Data(){
        type = DT_VAR;
        data = "";
    }
};


class FanCScope {

public:

    static std::vector<FanCError> errors;

    FanCScope* scopeAbove;
    std::map<std::string, Data> values;

    FanCScope(std::map<std::string, Data> values = {}, FanCScope* scopeAbove = nullptr) : values(values), scopeAbove(scopeAbove) {};

    void define(std::string name, std::variant<int, bool, double, std::string> value, DataType type){
        values[name] = Data(type, value);
    }

    std::variant<int, bool, double, std::string> getValue(Token name) {
        if (values.find(name.lexeme) != values.end()){
            return values.at(name.lexeme).data;
        }
        else if (scopeAbove != nullptr){
            return scopeAbove -> getValue(name);
        }
        else {
            FanCScope::errors.push_back(
                FanCError(name, FanCError::RUNTIME, "Undefined variable '" + name.lexeme + "'!"));
            return -1;
        }
    }

    void assign(const Token &name, const Token &assigner, const std::variant<int, bool, double, std::string> &value);
    
    std::string stringify(const std::variant<int, bool, double, std::string> &value);

    bool checkNumberOperands(
        const std::variant<int, bool, double, std::string> &left,
        const std::variant<int, bool, double, std::string> &right,
        const Token &name
    );

    bool isTruthy(const std::variant<int, bool, double, std::string> &value);

    // Only use when you know for sure.
    double toFloat(const std::variant<int, bool, double, std::string> &value);

    int doubleToInt(const std::variant<int, bool, double, std::string> &value);

    // Only use when you know for sure.
    int toInt(const std::variant<int, bool, double, std::string> &value);

    DataType findType(const std::variant<int, bool, double, std::string> &value);
};
