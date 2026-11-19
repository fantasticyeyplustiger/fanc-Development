#pragma once

#include "Token.hpp"
#include "fancerror.hpp"
#include "fancexpr.hpp"
#include "fancstatement.hpp"
#include <vector>

class FanCParser {

public:
    std::vector<FanCStatement> parse(std::vector<Token> &tokens);

private:

    int current = 0;
    int loops = 0;

    std::vector<Token> tokens;
    std::vector<FanCError> errors;

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
