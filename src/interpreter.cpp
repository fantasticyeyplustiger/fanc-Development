#include "interpreter.hpp"
#include "token.hpp"
#include "fancerror.hpp"



std::variant<int, bool, double, std::string> Interpreter::visitVar(Variable expr){

}

std::variant<int, bool, double, std::string> Interpreter::visitGrouping(Grouping expr){
    return evaluate(expr.expr);
}

std::variant<int, bool, double, std::string> Interpreter::visitUnary(Unary expr){
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

    // Unary expressions only made if they have one of these operators...
    // No need for default case
}

std::variant<int, bool, double, std::string> Interpreter::visitBinary(Binary expr){
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
                errors.push_back(
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

    return NULL;
}

std::variant<int, bool, double, std::string> Interpreter::visitTernary(Ternary expr){
    auto condition = evaluate(expr.condition);

    return isTruthy(condition) ? evaluate(expr.correct) : evaluate(expr.incorrect);
}

std::string Interpreter::stringify(std::variant<int, bool, double, std::string> value){
    switch (findType(value)){
        DT_INT: return std::to_string( toInt(value) );
        DT_FLOAT: return std::to_string( toFloat(value) );
        DT_BOOL: return std::get<bool>(value) ? "true" : "false";
        DT_STR: return '"' + std::get<std::string>(value) + '"';
        OTHER: return "nil";
    }
}

double Interpreter::toFloat(std::variant<int, bool, double, std::string> value){

    if (findType(value) == DT_INT){
        int n = std::get<int>(value);
        return static_cast<double>(n);
    }

    return std::get<double>(value);
}

int Interpreter::doubleToInt(std::variant<int, bool, double, std::string> value){
    if (findType(value) == DT_FLOAT){
        return static_cast<int>( std::get<double>(value) );
    }
    
    return std::get<int>(value);
}

int Interpreter::toInt(std::variant<int, bool, double, std::string> value){
    return std::get<int>(value);
}

bool Interpreter::eitherIsFloat(std::variant<int, bool, double, std::string> left, std::variant<int, bool, double, std::string> right){
    DataType leftType = findType(left);
    DataType rightType = findType(right);

    return leftType == DT_FLOAT || rightType == DT_FLOAT;
}

bool Interpreter::checkNumberOperands(Token op, std::variant<int, bool, double, std::string> left, std::variant<int, bool, double, std::string> right){
    
    DataType leftType = findType(left);
    DataType rightType = findType(right);
    
    if ((leftType == DT_FLOAT || leftType == DT_INT) && (rightType == DT_FLOAT || rightType == DT_INT)){
        return true;
    }

    errors.push_back(FanCError(op, FanCError::RUNTIME, "Operands must be numbers!"));
    return false;
}

bool Interpreter::checkNumberOperand(Token op, std::variant<int, bool, double, std::string> value){
    DataType type = findType(value);

    if (type == DT_FLOAT || type == DT_INT) { return true; }

    errors.push_back(FanCError(op, FanCError::RUNTIME, "Operand must be number!"));
    return false;
}

bool Interpreter::isTruthy(std::variant<int, bool, double, std::string> value){
    DataType type = findType(value);

    switch (type){
        case DT_INT: return std::get<int>(value) != 0;
        case DT_FLOAT: return std::get<double>(value) != 0.0;
        case DT_BOOL: return std::get<bool>(value);
        case DT_STR: return !std::get<std::string>(value).empty();
        default: return false; // very likely null lol
    }
}

DataType Interpreter::findType(std::variant<int, bool, double, std::string> value){

    switch (value.index()) {
        case DT_INT: return DT_INT;
        case DT_BOOL: return DT_BOOL;
        case DT_FLOAT: return DT_FLOAT;
        case DT_STR: return DT_STR;
        default: return OTHER;
    }
}
