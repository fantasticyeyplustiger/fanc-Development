#include "fancparser.hpp"
#include "fancerror.hpp"
#include "fancexpr.hpp"

FanCExpr FanCParser::ternary(){
    FanCExpr expr = orExpr();

    while (check(Type::TERNARY)) {
        Token op = previous();
        nextIsOperator(false);
        FanCExpr left = orExpr();
        
        if (!check(Type::COLON)){
            Token currentToken = peek();
            errors.push_back(FanCError(currentToken.lineNum, currentToken.column + 1, 1, FanCError::IGNORABLE,
            "Expected ':' for ternary operator!"));
        }
        else{
            advance(); // Consumes colon
        }

        nextIsOperator(false);
        FanCExpr right = orExpr();
        
        expr = Ternary(op, expr, left, right);
    }

    return expr;
}

FanCExpr FanCParser::orExpr(){
    FanCExpr expr = unary();

    while (check(Type::OR)) {
        Token op = previous();
        nextIsOperator(false);
        FanCExpr right = andExpr();
        expr = Binary(op, expr, right);
    }

    return expr;
}

FanCExpr FanCParser::andExpr(){
    FanCExpr expr = equality();

    while (check(Type::AND)) {
        Token op = previous();
        nextIsOperator(false);
        FanCExpr right = equality();
        expr = Binary(op, expr, right);
    }

    return expr;
}

FanCExpr FanCParser::equality(){
    FanCExpr expr = comparison();

    while (nextIsAnyIn({Type::NOT_EQUAL, Type::EQUAL_EQUAL})) {
        Token op = previous();
        nextIsOperator(false);
        FanCExpr right = comparison();
        expr = Binary(op, expr, right);
    }

    return expr;
}

FanCExpr FanCParser::comparison(){
    FanCExpr expr = term();

    while (nextIsAnyIn({Type::LESS, Type::LESS_EQUAL, Type::GREATER, Type::GREATER_EQUAL})) {
        Token op = previous();
        nextIsOperator(false);
        FanCExpr right = term();
        expr = Binary(op, expr, right);
    }

    return expr;
}

FanCExpr FanCParser::term(){
    FanCExpr expr = factor();

    while (nextIsAnyIn({Type::PLUS, Type::MINUS})) {
        Token op = previous();
        nextIsOperator(false);
        FanCExpr right = factor();
        expr = Binary(op, expr, right);
    }

    return expr;
}

FanCExpr FanCParser::factor(){
    FanCExpr expr = unary();

    while (nextIsAnyIn({Type::STAR, Type::SLASH, Type::MODULO})) {
        Token op = previous();
        nextIsOperator(false);
        FanCExpr right = unary();
        expr = Binary(op, expr, right);
    }

    return expr;
}

FanCExpr FanCParser::unary(){

    if (nextIsAnyIn({Type::NOT, Type::MINUS, Type::PLUS})){
        Token op = previous();
        FanCExpr right = unary();

        return Unary(op, right);
    }

    return primary();
}

FanCExpr FanCParser::primary(){

    FanCExpr expr = Literal(false);
    Token currentToken = peek();
    Type currentType = currentToken.type;
    bool initialized = false;

    switch (currentType){

        case Type::FALSE: expr = Literal(false); initialized = true; break;
        case Type::TRUE: expr = Literal(true); initialized = true; break;

        case Type::NUMBER:
        case Type::STRING: expr = Literal(currentToken.literal); initialized = true; break;

        case Type::IDENTIFIER: expr = Variable(currentToken); initialized = true; break;

        case Type::LEFT_PAREN: {
            advance();
            expr = expression();
            consume(Type::RIGHT_PAREN, "Expected ')' after expression!");
            return Grouping(expr);
        }

        case Type::RIGHT_BRACE: {
            errors.push_back(FanCError(currentToken, FanCError::COMPILER, "Unexpected '}'!"));
            break;
        }

        //default: {}
    }

    if (!initialized) {
        errors.push_back(FanCError(currentToken, FanCError::COMPILER,"Expected a literal!"));
        advance();
        return expr;
    }

    advance();

    if (nextIsAnyIn({Type::SEMICOLON, Type::LEFT_BRACE})) { return expr; }

    nextIsOperator();

    return expr;
}

bool FanCParser::consume(Type type, std::string errorMessage){
    if (check(type)) { advance(); return true; }
    
    errors.push_back(FanCError(previous(), FanCError::COMPILER, errorMessage));
    return false;
}

bool FanCParser::consumePrevious(Type type, std::string errorMessage){
    if (previous().type == type) { advance(); return true; }
    
    errors.push_back(FanCError(previous(), FanCError::COMPILER, errorMessage));
    return false;
}

bool FanCParser::nextIsAnyIn(std::vector<Type> types){

    for (Type type : types){
        if (check(type)){
            advance();
            return true;
        }
    }

    return false;
}
