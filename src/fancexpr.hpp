#pragma once

#include "token.hpp"
#include "interpreter.hpp"
#include <variant>
#include <string>

class Interpreter;

enum ExprIdentity {
    IGNORE,
    VARIABLE,
    NOT_INITIALIZED
};

class FanCExpr {

public:

    ExprIdentity identity = IGNORE;

    // Fundamentally returns a Literal.
    virtual std::variant<int, bool, double, std::string> accept(Interpreter* interpreter) const { return 0; };
    virtual Token getToken() const {return Token(0, 0, END, "", 0);};
    virtual ~FanCExpr() = default;
};

class Assign : public FanCExpr {
std::variant<int, bool, double, std::string> accept(Interpreter* interpreter) const { return interpreter -> visitAssign(*this);};

public:
    Token name;
    Token assigner;
    FanCExpr newValue;

    Assign(Token &name, Token &assigner, FanCExpr newValue) : name(name), assigner(assigner), newValue(newValue) {}
};


class Variable : public FanCExpr {

std::variant<int, bool, double, std::string> accept(Interpreter* interpreter) const { return interpreter -> visitVar(*this);};
ExprIdentity identity = VARIABLE;

public:
    Token identifier;
    Variable(Token &identifier) : identifier(identifier){};

    Token getToken() { return identifier; };
};


//class Call : public FanCExpr {
//std::variant<int, bool, double, std::string> accept(Interpreter* interpreter) const { return interpreter -> visitCall(*this);};
//};


class Literal : public FanCExpr {

std::variant<int, bool, double, std::string> accept(Interpreter* interpreter) const { return literal; };

public:
    std::variant<int, bool, double, std::string> literal;
    Literal(std::variant<int, bool, double, std::string> literal): literal(literal) {}
};


class Grouping : public FanCExpr {

std::variant<int, bool, double, std::string> accept(Interpreter* interpreter) const { return interpreter -> visitGrouping(*this);};

public:
    FanCExpr expr;
    Grouping(FanCExpr expr) : expr(expr) {};
};


class Unary : public FanCExpr {
std::variant<int, bool, double, std::string> accept(Interpreter* interpreter) const { return interpreter -> visitUnary(*this);};

public:
    Token op;
    FanCExpr right;
    Unary(Token &op, FanCExpr right): op(op), right(right) {}
};


class Binary : public FanCExpr {
std::variant<int, bool, double, std::string> accept(Interpreter* interpreter) const { return interpreter -> visitBinary(*this);};

public:
    Token op;
    FanCExpr left;
    FanCExpr right;
    Binary(Token &op, FanCExpr left, FanCExpr right): op(op), left(left), right(right) {}
};

class Ternary : public FanCExpr {
std::variant<int, bool, double, std::string> accept(Interpreter* interpreter) const { return interpreter -> visitTernary(*this);};

public:
    Token op;
    FanCExpr condition;
    FanCExpr correct;
    FanCExpr incorrect;
    Ternary(Token &op, FanCExpr condition, FanCExpr correct, FanCExpr incorrect):
    op(op), condition(condition), correct(correct), incorrect(incorrect) {}
};

