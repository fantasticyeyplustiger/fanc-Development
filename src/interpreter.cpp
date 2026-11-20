#include "interpreter.hpp"
#include "fancexpr.hpp"
#include "fancstatement.hpp"
#include "godot_cpp/core/print_string.hpp"
#include "token.hpp"
#include "fancerror.hpp"
#include <vector>

std::vector<FanCError> Interpreter::errors = {};

void Interpreter::interpret(std::vector<FanCStatement> statements){
    for (FanCStatement statement : statements){
        execute(statement);
    }
}

void Interpreter::execute(FanCStatement statement){
    statement.accept(this);
}

void Interpreter::executeBlock(std::vector<FanCStatement> &statements, FanCScope scope){
    FanCScope previousScope = currentScope;

    currentScope = scope;

    for (FanCStatement statement : statements){
        if (breaking || continuing){
            break;
        }

        if (statement.identity == ST_BREAK){
            loopCount--;
            breaking = true;
            break;
        }

        else if (statement.identity == ST_CONTINUE){
            continuing = true;
            break;
        }

        execute(statement);
    }

    currentScope = previousScope;
}

void Interpreter::visitForStmt(ForStmt statement){
    int priorLoopCount = loopCount;

    loopCount++;

    while (loopCount > priorLoopCount && isTruthy(evaluate(*statement.loop.condition))){
        execute(statement.loop.body);
        execute(statement.increment);
        continuing = false;
    }

    loopCount--;
    breaking = false;
}

void Interpreter::visitWhileStmt(WhileStmt statement){
    int priorLoopCount = loopCount;

    loopCount++;

    while (loopCount > priorLoopCount && isTruthy(evaluate(*statement.condition))){
        execute(statement.body);
        continuing = false;
    }

    loopCount--;
    breaking = false;
}

void Interpreter::visitIfStmt(IfStmt statement){
    for (int i = 0; i < statement.ifConditions.size(); i++){

        if (isTruthy(evaluate(statement.ifConditions.at(i)))){
            execute(statement.ifBranches.at(i));
            return;
        }

    }

    execute(statement.elseBranch);
}

void Interpreter::visitBlock(BlockStmt block){
    executeBlock(block.statements, FanCScope(currentScope));
}

void Interpreter::visitExprStmt(ExprStmt statement){
    evaluate(*statement.expr);
}

void Interpreter::visitPrintStmt(PrintStmt statement){
    auto value = stringify(evaluate(*statement.expr));
    godot::print_line(godot::String(value.c_str()));
}


void Interpreter::visitVarStmt(VariableStmt statement){
    Type type = statement.type.type;
    DataType typeForScope = DT_VAR;

    
    auto value = evaluate(*statement.initializer);

    switch (type) {
        case INT: typeForScope = DT_INT; break;
        case FLOAT: typeForScope = DT_FLOAT; break;
        case BOOL: typeForScope = DT_BOOL; break;
        case STR: typeForScope = DT_STR;
        // default is DT_VAR
    }
    
    currentScope.define(statement.name.lexeme, value, typeForScope);

}

std::variant<int, bool, double, std::string> Interpreter::evaluate(const FanCExpr &expr){
    return expr.accept(this);
}

std::variant<int, bool, double, std::string> Interpreter::visitAssign(const Assign &expr){
    auto value = evaluate(expr.newValue);
    currentScope.assign(expr.name, expr.assigner, value);
    return value;
}

std::variant<int, bool, double, std::string> Interpreter::visitVar(const Variable &expr){
    return currentScope.getValue(expr.identifier);
}

std::variant<int, bool, double, std::string> Interpreter::visitGrouping(const Grouping &expr){
    return evaluate(expr.expr);
}

std::variant<int, bool, double, std::string> Interpreter::visitUnary(const Unary &expr){
    auto right = evaluate(expr.right);

    switch (expr.op.type) {
        MINUS:
            if (checkNumberOperand(expr.op, right)) {
                if (findType(right) == DT_INT){
                    return -std::get<int>(right);
                }
                else{
                    return -std::get<double>(right);
                }
            }
            else {return right;}
        
        PLUS:
            checkNumberOperand(expr.op, right);
            return right;
        NOT:
            return !isTruthy(right);
    }

    return 0;

    // Unary expressions only made if they have one of these operators...
    // No need for default case
}

std::variant<int, bool, double, std::string> Interpreter::visitBinary(const Binary &expr){
    auto left = evaluate(expr.left);
    auto right = evaluate(expr.right);

    switch (expr.op.type) {
        case MINUS:
            if (checkNumberOperands(expr.op, left, right)){
                if (eitherIsFloat(left, right)){
                    return toFloat(left) - toFloat(right);
                }
                else { return toInt(left) - toInt(right); }
            }
        case STAR:
            if (checkNumberOperands(expr.op, left, right)){
                if (eitherIsFloat(left, right)){
                    return toFloat(left) * toFloat(right);
                }
                else { return toInt(left) * toInt(right); }
            }
        case SLASH:
            if (checkNumberOperands(expr.op, left, right)){
                if (eitherIsFloat(left, right)){
                    return toFloat(left) / toFloat(right);
                }
                else { return toInt(left) / toInt(right); }
            }
        case MODULO:
            if (checkNumberOperands(expr.op, left, right)){
                return doubleToInt(left) % doubleToInt(right);
            }
        
        case PLUS:
            if ((findType(left) == DT_STR) || (findType(right) == DT_STR)){
                return stringify(left) + stringify(right);
            }
            else if (eitherIsFloat(left, right)){
                return toFloat(left) + toFloat(right);
            }
            else if ((findType(left) == DT_INT) && (findType(right) == DT_INT)){
                return toInt(left) + toInt(right);
            }
            else {
                Interpreter::errors.push_back(
                    FanCError(expr.op, FanCError::RUNTIME,
                    "Operands must be two numbers or be strings!")
                );
                break;
            }
        
        case GREATER:
            if (checkNumberOperands(expr.op, left, right)){
                return left > right;
            }
            addComparisonError(expr.op);
            break;
        
        case GREATER_EQUAL:
            if (checkNumberOperands(expr.op, left, right)){
                return left >= right;
            }
            addComparisonError(expr.op);
            break;
        
        case LESS:
            if (checkNumberOperands(expr.op, left, right)){
                return left < right;
            }
            addComparisonError(expr.op);
            break;
        
        case LESS_EQUAL:
            if (checkNumberOperands(expr.op, left, right)){
                return left <= right;
            }
            addComparisonError(expr.op);
            break;
        
        case AND: return isTruthy(left) && isTruthy(right);
        case OR:  return isTruthy(left) || isTruthy(right);

        case NOT_EQUAL:   return !areEqual(left, right);
        case EQUAL_EQUAL: return areEqual(left, right);
    }

    return 0;
}

std::variant<int, bool, double, std::string> Interpreter::visitTernary(const Ternary &expr){
    auto condition = evaluate(expr.condition);

    return isTruthy(condition) ? evaluate(expr.correct) : evaluate(expr.incorrect);
}

std::string Interpreter::stringify(const std::variant<int, bool, double, std::string> &value){
    switch (findType(value)){
        case DT_INT: return std::to_string( toInt(value) );
        case DT_FLOAT: return std::to_string( toFloat(value) );
        case DT_BOOL: return std::get<bool>(value) ? "true" : "false";
        case DT_STR: return '"' + std::get<std::string>(value) + '"';
        default: return "nil";
    }
}

double Interpreter::toFloat(const std::variant<int, bool, double, std::string> &value){

    if (findType(value) == DT_INT){
        int n = std::get<int>(value);
        return static_cast<double>(n);
    }

    return std::get<double>(value);
}

int Interpreter::doubleToInt(const std::variant<int, bool, double, std::string> &value){
    if (findType(value) == DT_FLOAT){
        return static_cast<int>( std::get<double>(value) );
    }
    
    return std::get<int>(value);
}

int Interpreter::toInt(const std::variant<int, bool, double, std::string> &value){
    return std::get<int>(value);
}

bool Interpreter::eitherIsFloat(
    const std::variant<int, bool, double, std::string> &left,
    const std::variant<int, bool, double, std::string> &right) {
    
    DataType leftType = findType(left);
    DataType rightType = findType(right);

    return leftType == DT_FLOAT || rightType == DT_FLOAT;
}

bool Interpreter::checkNumberOperands(
    const Token &op,
    const std::variant<int, bool, double, std::string> &left,
    const std::variant<int, bool, double, std::string> &right){
    
    DataType leftType = findType(left);
    DataType rightType = findType(right);
    
    if ((leftType == DT_FLOAT || leftType == DT_INT) && (rightType == DT_FLOAT || rightType == DT_INT)){
        return true;
    }

    Interpreter::errors.push_back(FanCError(op, FanCError::RUNTIME, "Operands must be numbers!"));
    return false;
}

bool Interpreter::checkNumberOperand(
    const Token &op,
    const std::variant<int, bool, double, std::string> &value){

    DataType type = findType(value);

    if (type == DT_FLOAT || type == DT_INT) { return true; }

    Interpreter::errors.push_back(FanCError(op, FanCError::RUNTIME, "Operand must be number!"));
    return false;
}

bool Interpreter::isTruthy(const std::variant<int, bool, double, std::string> &value){
    DataType type = findType(value);

    switch (type){
        case DT_INT: return std::get<int>(value) != 0;
        case DT_FLOAT: return std::get<double>(value) != 0.0;
        case DT_BOOL: return std::get<bool>(value);
        case DT_STR: return !std::get<std::string>(value).empty();
        default: return false; // very likely null lol
    }
}

DataType Interpreter::findType(const std::variant<int, bool, double, std::string> &value){

    switch (value.index()) {
        case DT_INT: return DT_INT;
        case DT_BOOL: return DT_BOOL;
        case DT_FLOAT: return DT_FLOAT;
        case DT_STR: return DT_STR;
        default: return OTHER;
    }
}
