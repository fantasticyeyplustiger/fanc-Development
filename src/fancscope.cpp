#include "fancscope.hpp"
#include "fancerror.hpp"
#include <vector>

std::vector<FanCError> FanCScope::errors = {};

void FanCScope::assign(const Token &name, const Token &assigner, const std::variant<int, bool, double, std::string> &value){
    if (scopeAbove != nullptr){
            scopeAbove -> assign(name, assigner, value);
        }

    else if (values.find(name.lexeme) == values.end()) {
        FanCScope::errors.push_back(
            FanCError(name, FanCError::RUNTIME, "Undefined variable '" + name.lexeme + "'!"));
    }

    else{
        const auto assigned = values.at(name.lexeme).data;

        switch (assigner.type) {

            case EQUAL:

                switch (values[name.lexeme].type){

                    case DT_INT:
                            
                        if (findType(value) == DT_FLOAT){
                            values[name.lexeme].data = toInt(value);
                        }
                        else if (findType(value) == DT_INT){
                            values[name.lexeme].data = value;
                        }
                        else{
                            FanCScope::errors.push_back(
                                FanCError(
                                    name, FanCError::RUNTIME, "Cannot assign this data type to an integer!"));
                        }
                        break;

                    case DT_FLOAT:
                            
                        if (findType(value) == DT_FLOAT){
                            values[name.lexeme].data = value;
                        }
                        else if (findType(value) == DT_INT){
                            values[name.lexeme].data = toFloat(value);
                        }
                        else{
                            FanCScope::errors.push_back(
                                FanCError(
                                    name, FanCError::RUNTIME, "Cannot assign this data type to a float!"));
                        }
                        break;
                        
                    case DT_STR:
                        values[name.lexeme].data = stringify(value); break;
                        
                    case DT_BOOL:
                        values[name.lexeme].data = isTruthy(value); break;
                        
                    case DT_VAR:
                        values[name.lexeme].data = value;
                }

            case PLUS_EQUAL:
                    
                switch (values[name.lexeme].type){

                    case DT_STR:
                        values[name.lexeme].data = stringify(assigned) + stringify(value);
                        break;
                        
                    case DT_INT:
                        if (findType(value) == DT_INT){
                            values[name.lexeme].data = toInt(assigned) + toInt(value);
                        }
                        else if (findType(value) == DT_FLOAT){
                            values[name.lexeme].data = toInt(assigned) + doubleToInt(value);
                        }
                        else{
                            FanCScope::errors.push_back(
                                FanCError(name,
                                FanCError::RUNTIME,
                                "Plus-Equaling incompatible data types! Not assigning..."));
                        }
                        
                    case DT_FLOAT:
                        if (findType(value) == DT_INT || findType(value) == DT_FLOAT){
                            values[name.lexeme].data = toFloat(assigned) + toFloat(value);
                            break;
                        }
                        
                    default:
                        FanCScope::errors.push_back(
                        FanCError(name,
                        FanCError::RUNTIME,
                        "Plus-Equaling incompatible data types! Not assigning..."));
                }
                break;
                
            case MINUS_EQUAL:
                    
                if (checkNumberOperands(assigned, value, name)){

                    if (values[name.lexeme].type == DT_INT){
                        values[name.lexeme].data = toInt(assigned) - doubleToInt(value);
                    }
                    else{
                        values[name.lexeme].data = toFloat(assigned) - toFloat(value);
                    }

                }
                break;
                
            case STAR_EQUAL:

                if (checkNumberOperands(assigned, value, name)){

                    if (values[name.lexeme].type == DT_INT){
                        values[name.lexeme].data = toInt(assigned) * doubleToInt(value);
                    }
                    else{
                        values[name.lexeme].data = toFloat(assigned) * toFloat(value);
                    }

                }
                break;
                
            case SLASH_EQUAL:

                if (checkNumberOperands(assigned, value, name)){

                    if (values[name.lexeme].type == DT_INT){
                        values[name.lexeme].data = toInt(assigned) / doubleToInt(value);
                    }
                    else{
                        values[name.lexeme].data = toFloat(assigned) / toFloat(value);
                    }

                }
                break;
                
            case MODULO_EQUAL:

                if (checkNumberOperands(assigned, value, name)){

                    if (values[name.lexeme].type == DT_INT){
                        values[name.lexeme].data = toInt(assigned) % doubleToInt(value);
                    }
                    else{
                        FanCScope::errors.push_back(
                            FanCError(name, FanCError::RUNTIME, "Can't use modulo assigner on a float!"));
                    }

                }
                break;
        }

    }
}


std::string FanCScope::stringify(const std::variant<int, bool, double, std::string> &value){
    switch (findType(value)){
        case DT_INT: return std::to_string( toInt(value) );
        case DT_FLOAT: return std::to_string( toFloat(value) );
        case DT_BOOL: return std::get<bool>(value) ? "true" : "false";
        case DT_STR: return std::get<std::string>(value);
        case DT_VAR:
        case OTHER: return "nil";
    }
}

bool FanCScope::isTruthy(const std::variant<int, bool, double, std::string> &value){
    DataType type = findType(value);

    switch (type){
        case DT_INT: return std::get<int>(value) != 0;
        case DT_FLOAT: return std::get<double>(value) != 0.0;
        case DT_BOOL: return std::get<bool>(value);
        case DT_STR: return !std::get<std::string>(value).empty();
        default: return false; // very likely null lol
    }
}

bool FanCScope::checkNumberOperands(
    const std::variant<int, bool, double, std::string> &left,
    const std::variant<int, bool, double, std::string> &right,
    const Token &name){
    
    const DataType leftType = findType(left);
    const DataType rightType = findType(right);
    
    if ((leftType == DT_FLOAT || leftType == DT_INT) && (rightType == DT_FLOAT || rightType == DT_INT)){
        return true;
    }

    FanCScope::errors.push_back(FanCError(name, FanCError::RUNTIME,
    "Assigning incorrect data type! Expression not evaluated..."));
    return false;
}

double FanCScope::toFloat(const std::variant<int, bool, double, std::string> &value){

    if (findType(value) == DT_INT){
        int n = std::get<int>(value);
        return static_cast<double>(n);
    }

    return std::get<double>(value);
}

int FanCScope::doubleToInt(const std::variant<int, bool, double, std::string> &value){
    if (findType(value) == DT_FLOAT){
        return static_cast<int>( std::get<double>(value) );
    }
    
    return std::get<int>(value);
}

int FanCScope::toInt(const std::variant<int, bool, double, std::string> &value){
    return std::get<int>(value);
}

DataType FanCScope::findType(const std::variant<int, bool, double, std::string> &value){

    switch (value.index()) {
        case DT_INT: return DT_INT;
        case DT_BOOL: return DT_BOOL;
        case DT_FLOAT: return DT_FLOAT;
        case DT_STR: return DT_STR;
        default: return DT_VAR;
    }
}
