#pragma once

#include "Token.hpp"
#include "fancerror.hpp"
#include "fancexpr.hpp"
#include "fancstatement.hpp"
#include <vector>

class FanCParser {

public:
    void parse(std::vector<Token> &tokens);
    std::vector<FanCStatement> statements;
    std::vector<FanCError> errors;

private:

    int current = 0;
    int loops = 0;

    const Type OPERATORS[22] = {
        GREATER, GREATER_EQUAL, LESS, LESS_EQUAL,
        EQUAL_EQUAL, NOT_EQUAL,
        PLUS, MINUS, STAR, SLASH,
        MODULO, TERNARY, COLON,
        AND, OR, RIGHT_PAREN,
        EQUAL, PLUS_EQUAL, MINUS_EQUAL,
        STAR_EQUAL, SLASH_EQUAL, MODULO_EQUAL
    };

    std::vector<Token> tokens;

    FanCStatement declaration();
    FanCStatement varDeclaration();
    FanCStatement statement();
    FanCStatement forStatement();
    FanCStatement whileStatement();
    FanCStatement breakStatement();
    FanCStatement continueStatement();
    FanCStatement ifStatement();
    FanCStatement printStatement();
    FanCStatement expressionStatement();
    
    BlockStmt block();

    FanCExpr expression() { return assignment(); };
    FanCExpr assignment();
    FanCExpr ternary();
    FanCExpr orExpr();
    FanCExpr andExpr();
    FanCExpr equality();
    FanCExpr comparison();
    FanCExpr term();
    FanCExpr factor();
    FanCExpr unary();
    FanCExpr primary();

    void synchronize();

    bool validErrors();
    bool nextIsOperator(bool expectingOp = true);
    bool nextIsAnyIn(std::vector<Type> types);
    bool isEnd(){ return tokens.at(current).type == Type::END;};
    bool check(Type type) { return isEnd() ? false : tokens.at(current).type == type; }

    bool consume(Type type, std::string errorMessage);
    bool consumePrevious(Type type, std::string errorMessage);

    Token advance(){
        if (!isEnd()) { current++; }
        return previous();
    };
    Token peek(){ return tokens.at(current); }
    Token previous(){ return current <= 0 ? tokens.at(0) : tokens.at(current - 1); }

};
