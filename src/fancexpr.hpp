#pragma once

#include "token.hpp"
#include "interpreter.hpp"
#include <variant>
#include <string>

enum ExprIdentity {
    IGNORE,
    VARIABLE
};

class FanCExpr {

public:
    // Fundamentally returns a Literal.
    virtual std::variant<int, bool, double, std::string> accept(Interpreter interpreter);
    virtual Token getToken();
    virtual ~FanCExpr() = default;

    ExprIdentity getIdentity() { return ExprIdentity::IGNORE; };
};

class Assign : public FanCExpr {
std::variant<int, bool, double, std::string> accept(Interpreter interpreter);

public:
    Token name;
    Token assigner;
    FanCExpr newValue;

    Assign(Token name, Token assigner, FanCExpr newValue) : name(name), assigner(assigner), newValue(newValue) {}
};


class Variable : public FanCExpr {

std::variant<int, bool, double, std::string> accept(Interpreter interpreter);

public:
    Token identifier;
    Variable(Token identifier) : identifier(identifier){};

    Token getToken() { return identifier; };
    ExprIdentity getIdentity() { return VARIABLE; }
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

