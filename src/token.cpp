#include "token.hpp"
#include <unordered_map>

std::string Token::toString(){
    std::string where = "[" + std::to_string(lineNum) + ", " + std::to_string(column) + "]";
    std::string t = "Type: " + std::to_string(type);
    return where + " " + t + ", Lexeme: " + lexeme;
}

const std::unordered_map<std::string, Type> Token::Keywords = {
    {"int", INT},
    {"float", FLOAT},
    {"bool", BOOL},
    {"str", STR},
    {"var", VAR},
    {"true", TRUE},
    {"false", FALSE},
    {"if", IF},
    {"elif", ELIF},
    {"else", ELSE},
    {"and", AND},
    {"or", OR},
    {"not", NOT},
    {"print", PRINT},
    {"return", RETURN},
    {"break", BREAK},
    {"continue", CONTINUE},
    {"while", WHILE},
    {"for", FOR},
    {"super", SUPER},
    {"class", CLASS}
};

Type Token::getKeyword(std::string keyword){
    return Keywords.at(keyword);
}

bool Token::isKeyword(std::string keyword){
    auto it = Keywords.find(keyword);
    return it != Keywords.end();
}
