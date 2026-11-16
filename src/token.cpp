#include "token.hpp"

std::string Token::toString(){
    std::string where = "[" + std::to_string(lineNum) + ", " + std::to_string(column) + "]";
    std::string t = "Type: " + std::to_string(type);
    return where + " " + t + ", Lexeme: " + lexeme;
}
