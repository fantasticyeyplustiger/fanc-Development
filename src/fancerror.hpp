#pragma once
#include "token.hpp"
#include <string>

class FanCError {

public:

    enum Type {
        COMPILER,
        RUNTIME,
        IGNORABLE
    };
    
    int lineNum;
    int column;
    int length;
    Type type;
    std::string message;

    FanCError(int lineNum, int column, int length, Type type, std::string message) :
        lineNum(lineNum), column(column), length(length), type(type), message(message) {};
    
    FanCError(Token token, Type type, std::string message) :
        type(type), message(message) {
            lineNum = token.lineNum;
            column = token.column;
            length = token.lexeme.length();
        };
};
