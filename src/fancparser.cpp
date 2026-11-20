#include "fancparser.hpp"
#include "Token.hpp"
#include "fancerror.hpp"
#include "fancexpr.hpp"
#include "fancstatement.hpp"
#include <vector>

void FanCParser::parse(std::vector<Token> &tokens){
    this -> tokens = tokens;
    
    std::vector<FanCStatement> statements;

    current = 0;
    FanCParser::errors = {};

    while (!isEnd()){
        statements.push_back(declaration());
    }

    this -> statements = statements;
}


FanCStatement FanCParser::declaration(){
    if (nextIsAnyIn({VAR, INT, FLOAT, BOOL, STR})){
        return varDeclaration();
    }

    FanCStatement statement = FanCParser::statement();

    if (FanCParser::errors.size() > 0){
        synchronize();
    }

    return statement;
}

FanCStatement FanCParser::varDeclaration(){
    Token type = previous();
    consume(IDENTIFIER, "Expected variable name!");
    Token varName = previous();

    FanCExpr initializer;

    if (check(EQUAL)) { advance(); initializer = expression(); }

    else {
        switch (type.type){
            INT: initializer = Literal(0);
            FLOAT: initializer = Literal(0.0);
            BOOL: initializer = Literal(false);
            VAR:
            STR: initializer = Literal("");
            default:
                FanCParser::errors.push_back(FanCError(type, FanCError::COMPILER, "Could not declare variable..."));
                initializer = Literal("");
        }
    }

    FanCExpr* pointer = &initializer;

    consumePrevious(SEMICOLON, "Expected ';' after variable declaration!");
    return VariableStmt(type, varName, pointer);
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

FanCStatement FanCParser::forStatement(){
    consume(LEFT_PAREN, "Expected '(' after 'for'.");

    FanCStatement initializer;

    if (nextIsAnyIn({INT, FLOAT, BOOL, STR, VAR})){
        initializer = varDeclaration();
    }
    else{
        initializer = expressionStatement();
    }

    FanCExpr condition;

    if (!nextIsAnyIn({SEMICOLON})){
        condition = expression();
    }

    consume(SEMICOLON, "Expected ';' after for loop condition!");

    FanCExpr increment;

    if (!nextIsAnyIn({RIGHT_PAREN})){
        increment = expression();
    }

    if (nextIsAnyIn({SEMICOLON})) { advance(); }

    consume(RIGHT_PAREN, "Expected ')' after for clauses!");
    advance();
    
    loops++;

    BlockStmt body;
    Token prev = previous();

    if (prev.type == LEFT_BRACE){
        body = block();
    }
    else{
        FanCParser::errors.push_back(
            FanCError(
            prev.lineNum, prev.column + 1, 1, FanCError::COMPILER,
            "Expected '{' after for's ')'!")
        );
    }

    if (condition.identity == NOT_INITIALIZED){
        condition = Literal(true);
    }

    loops--;

    FanCExpr* conditionPointer = &condition;

    auto loop = WhileStmt(conditionPointer, body);


    if (increment.identity != NOT_INITIALIZED){

        auto incrementPointer = &increment;

        auto finalBody = ForStmt(ExprStmt(incrementPointer), initializer, loop);

        return finalBody;
    }

    return loop;
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
        FanCParser::errors.push_back(
                FanCError(
                    prev.lineNum, prev.column + 1, 1, FanCError::COMPILER,
                    "Expected '{' after while condition!"));
            body = BlockStmt({}, NONEXISTENT);
    }

    loops--;

    FanCExpr* pointer = &condition;

    return WhileStmt(pointer, body);
}


FanCStatement FanCParser::breakStatement(){
    if (loops == 0){
        FanCParser::errors.push_back(FanCError(peek(), FanCError::COMPILER, "'break' used outside loop!"));
    }

    advance();
    if (check(SEMICOLON)) {advance();} // Doesn't need to be there

    return BreakStmt();
}

FanCStatement FanCParser::continueStatement(){
    if (loops == 0){
        FanCParser::errors.push_back(FanCError(peek(), FanCError::COMPILER, "'continue' used outside loop!"));
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
            FanCParser::errors.push_back(
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
            FanCParser::errors.push_back(FanCError(prev.lineNum, prev.column + 1, 1, FanCError::COMPILER,
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
        FanCParser::errors.push_back(
            FanCError(previous(), FanCError::COMPILER, "Expected ';' after expression!"));
    }
    advance(); // consume

    FanCExpr* pointer = &value;

    return PrintStmt(pointer);
}


FanCStatement FanCParser::expressionStatement(){
    FanCExpr value = expression();

    if (!check(SEMICOLON)) {
        FanCParser::errors.push_back(
            FanCError(previous(), FanCError::COMPILER, "Expected ';' after expression!"));
    }
    advance(); // consume

    FanCExpr* pointer = &value;

    return ExprStmt(pointer);
}


FanCExpr FanCParser::assignment(){
    FanCExpr expr = ternary();

    if (nextIsAnyIn(
        {EQUAL, PLUS_EQUAL, MINUS_EQUAL, STAR_EQUAL, SLASH_EQUAL, MODULO_EQUAL})) {

        Token equals = previous();
        FanCExpr value = assignment();

        if (expr.identity == VARIABLE){
            Token identifier = expr.getToken();
            return Assign(identifier, equals, value);
        }
        else {
            FanCParser::errors.push_back(FanCError(equals, FanCError::COMPILER, "Invalid assignment target!"));
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
            FanCParser::errors.push_back(FanCError(currentToken.lineNum, currentToken.column + 1, 1, FanCError::IGNORABLE,
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
            FanCParser::errors.push_back(FanCError(currentToken, FanCError::COMPILER, "Unexpected '}'!"));
            break;
        }

        //default: {}
    }

    if (!initialized) {
        FanCParser::errors.push_back(FanCError(currentToken, FanCError::COMPILER,"Expected a literal!"));
        advance();
        expr.identity = NOT_INITIALIZED;
        return expr;
    }

    advance();

    if (nextIsAnyIn({Type::SEMICOLON, Type::LEFT_BRACE})){
        return expr;
    }

    nextIsOperator();

    return expr;
}

void FanCParser::synchronize(){
    advance();

    while (!isEnd()){
        if (previous().type == SEMICOLON) { return; }

        switch (peek().type){
            case CLASS:
            case VAR:
            case INT:
            case FLOAT:
            case STR:
            case BOOL:
            case IF:
            case ELIF:
            case ELSE:
            case WHILE:
            case PRINT:
            case RETURN: return;
            default: {}
        }

        advance();
    }
}

bool FanCParser::consume(Type type, std::string errorMessage){
    if (check(type)) { advance(); return true; }
    
    peek().print();

    Token prev = previous();
    FanCParser::errors.push_back(FanCError(prev.lineNum, prev.column + 1, 1, FanCError::COMPILER, errorMessage));
    return false;
}

bool FanCParser::consumePrevious(Type type, std::string errorMessage){
    if (previous().type == type) { advance(); return true; }
    
    FanCParser::errors.push_back(FanCError(previous(), FanCError::COMPILER, errorMessage));
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

bool FanCParser::nextIsOperator(bool expectingOp){
    if (isEnd()) { return false; }

    Token op = peek();

    for (Type valid : OPERATORS){
        if (op.type == valid && !expectingOp){
            FanCParser::errors.push_back(
                FanCError(op, FanCError::COMPILER, "Shouldn't have more than one operator!"));
        }
        else if (op.type == valid) {
            return true;
        }
    }

    if (expectingOp){
        FanCParser::errors.push_back(
            FanCError(op, FanCError::COMPILER, "Expected an operator!"));
    }

    return false;
}
