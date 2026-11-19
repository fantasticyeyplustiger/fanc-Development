#include "fancparser.hpp"
#include "Token.hpp"
#include "fancerror.hpp"
#include "fancexpr.hpp"
#include "fancstatement.hpp"
#include <vector>

std::vector<FanCStatement> FanCParser::parse(std::vector<Token> &tokens){
    this -> tokens = tokens;
    
    std::vector<FanCStatement> statements;

    current = 0;
    errors = {};

    while (!isEnd()){
        statements.push_back(declaration());
    }

    return statements;
}


FanCStatement FanCParser::declaration(){
    if (nextIsAnyIn({VAR, INT, FLOAT, BOOL, STR})){
        return varDeclaration();
    }

    FanCStatement statement = FanCParser::statement();

    if (errors.size() > 0){
        synchronize();
    }

    return statement;
}

FanCStatement FanCParser::varDeclaration(){
    Token type = previous();
    consume(IDENTIFIER, "Expected variable name!");
    Token varName = previous();

    FanCExpr initializer;

    if (check(EQUAL)) { initializer = expression(); }

    else {
        switch (type.type){
            INT: initializer = Literal(0);
            FLOAT: initializer = Literal(0.0);
            BOOL: initializer = Literal(false);
            VAR:
            STR: initializer = Literal("");
            default:
                errors.push_back(FanCError(type, FanCError::COMPILER, "Could not declare variable..."));
                initializer = Literal("");
        }
    }

    consume(SEMICOLON, "Expected ';' after variable declaration!");
    return VariableStmt(type, varName, initializer);
}


FanCStatement FanCParser::statement(){
    
    switch (peek().type){
        IF:         advance(); return ifStatement();
        BREAK:      advance(); return breakStatement();
        CONTINUE:   advance(); return continueStatement();
        PRINT:      advance(); return printStatement();
        WHILE:      advance(); return whileStatement();
        //FOR:        advance(); return forStatement();
        LEFT_BRACE: advance(); return block();
        default: {}
    }

    return expressionStatement();
}


BlockStmt FanCParser::block(){
    std::vector<FanCStatement> statements = {};

    while (!check(RIGHT_BRACE) and !isEnd()){
        statements.push_back(declaration());
    }

    consume(RIGHT_BRACE, "Expected '}' after block!");
    return BlockStmt(statements);
}


FanCStatement FanCParser::whileStatement(){
    FanCExpr condition = expression();
    BlockStmt body;
    Token prev = previous();

    loops++;

    if (previous().type == LEFT_BRACE){
        body = block();
    }
    else if (peek().type == LEFT_BRACE){
        advance();
        body = block();
    }
    else{
        errors.push_back(
                FanCError(
                    prev.lineNum, prev.column + 1, 1, FanCError::COMPILER,
                    "Expected '{' after while condition!"));
            body = BlockStmt({}, NONEXISTENT);
    }

    loops--;

    return WhileStmt(condition, body);
}


FanCStatement FanCParser::breakStatement(){
    if (loops == 0){
        errors.push_back(FanCError(peek(), FanCError::COMPILER, "'break' used outside loop!"));
    }

    advance();
    if (check(SEMICOLON)) {advance();} // Doesn't need to be there

    return BreakStmt();
}

FanCStatement FanCParser::continueStatement(){
    if (loops == 0){
        errors.push_back(FanCError(peek(), FanCError::COMPILER, "'continue' used outside loop!"));
    }

    advance();
    if (check(SEMICOLON)) {advance();} // Doesn't need to be there

    return ContinueStmt();
}


FanCStatement FanCParser::ifStatement(){
    
    std::vector<FanCExpr> conditions = {};
    std::vector<BlockStmt> branches = {};

    do {
        FanCExpr condition = expression();
        BlockStmt then;
        Token prev = previous(); // right after expression

        if (prev.type == LEFT_BRACE) {
            then = block();
        }
        else if (peek().type == LEFT_BRACE) {
            advance(); // ignores previous
            then = block();
        }
        else{
            errors.push_back(
                FanCError(
                    prev.lineNum, prev.column + 1, 1, FanCError::COMPILER,
                    "Expected '{' after if condition!"));
            then = BlockStmt({}, NONEXISTENT);
        }

        conditions.push_back(condition);
        branches.push_back(then);
    }
    while (check(ELIF));

    BlockStmt elseBranch;

    if (check(ELSE)){
        advance();
        Token prev = previous();
        if (prev.type != LEFT_BRACE){
            errors.push_back(FanCError(prev.lineNum, prev.column + 1, 1, FanCError::COMPILER,
                "Expected '{' after else!"));
        }
        elseBranch = block();
    }
    else {
        elseBranch = BlockStmt({}, NONEXISTENT);
    }

    if (conditions.empty()) {
        return IfStmt(conditions, branches, elseBranch, NONEXISTENT);
    }

    return IfStmt(conditions, branches, elseBranch);
}


FanCStatement FanCParser::printStatement(){
    FanCExpr value = expression();

    if (!check(SEMICOLON)) {
        errors.push_back(
            FanCError(previous(), FanCError::COMPILER, "Expected ';' after expression!"));
    }
    advance(); // consume

    return PrintStmt(value);
}


FanCStatement FanCParser::expressionStatement(){
    FanCExpr value = expression();

    if (!check(SEMICOLON)) {
        errors.push_back(
            FanCError(previous(), FanCError::COMPILER, "Expected ';' after expression!"));
    }
    advance(); // consume

    return ExprStmt(value);
}


FanCExpr FanCParser::assignment(){
    FanCExpr expr = ternary();

    if (nextIsAnyIn(
        {EQUAL, PLUS_EQUAL, MINUS_EQUAL, STAR_EQUAL, SLASH_EQUAL, MODULO_EQUAL})) {

        Token equals = previous();
        FanCExpr value = assignment();

        if (expr.getIdentity() == VARIABLE){
            Token identifier = expr.getToken();
            return Assign(identifier, equals, value);
        }
        else {
            errors.push_back(FanCError(equals, FanCError::COMPILER, "Invalid assignment target!"));
        }
    }

    return expr;
}

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
