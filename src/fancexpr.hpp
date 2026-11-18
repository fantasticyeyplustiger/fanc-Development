#pragma once

#include "token.hpp"
#include "interpreter.hpp"
#include <variant>
#include <string>

class FanCExpr {

public:
    // accept() fundamentally returns a Literal.
    virtual std::variant<int, bool, double, std::string> accept(Interpreter interpreter);
    virtual ~FanCExpr() = default;
};

class Assign : public FanCExpr {
std::variant<int, bool, double, std::string> accept(Interpreter interpreter);
};


class Variable : public FanCExpr {

std::variant<int, bool, double, std::string> accept(Interpreter interpreter);

public:
    Token identifier;
    Variable(Token identifier) : identifier(identifier){};
};


class Call : public FanCExpr {
std::variant<int, bool, double, std::string> accept(Interpreter interpreter);
};


class Literal : public FanCExpr {

std::variant<int, bool, double, std::string> accept(Interpreter interpreter);

public:
    std::variant<int, bool, double, std::string> literal;
    Literal(std::variant<int, bool, double, std::string> literal): literal(literal) {}
};


class Grouping : public FanCExpr {

std::variant<int, bool, double, std::string> accept(Interpreter interpreter);

public:
    FanCExpr expr;
    Grouping(FanCExpr expr) : expr(expr) {};
};


class Unary : public FanCExpr {
std::variant<int, bool, double, std::string> accept(Interpreter interpreter);

public:
    Token op;
    FanCExpr right;
    Unary(Token op, FanCExpr right): op(op), right(right) {}
};


class Binary : public FanCExpr {
std::variant<int, bool, double, std::string> accept(Interpreter interpreter);

public:
    Token op;
    FanCExpr left;
    FanCExpr right;
    Binary(Token op, FanCExpr left, FanCExpr right): op(op), left(left), right(right) {}
};

class Ternary : public FanCExpr {
std::variant<int, bool, double, std::string> accept(Interpreter interpreter);

public:
    Token op;
    FanCExpr condition;
    FanCExpr correct;
    FanCExpr incorrect;
    Ternary(Token op, FanCExpr condition, FanCExpr correct, FanCExpr incorrect):
    op(op), condition(condition), correct(correct), incorrect(incorrect) {}
};

