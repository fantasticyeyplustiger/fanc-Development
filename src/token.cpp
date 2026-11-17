#include "token.hpp"
#include <unordered_map>

std::string Token::toString(){
    std::string where = "[" + std::to_string(lineNum) + ", " + std::to_string(column) + "]";
    std::string t = "Type: " + std::to_string(type);
    return where + " " + t + ", Lexeme: " + lexeme;
}

const std::unordered_map<std::string, Token::Type> Token::Keywords = {
    {"int", Token::Type::INT},
    {"float", Token::Type::FLOAT},
    {"bool", Token::Type::BOOL},
    {"str", Token::Type::STR},
    {"var", Token::Type::VAR},
    {"true", Token::Type::TRUE},
    {"false", Token::Type::FALSE},
    {"if", Token::Type::IF},
    {"elif", Token::Type::ELIF},
    {"else", Token::Type::ELSE},
    {"and", Token::Type::AND},
    {"or", Token::Type::OR},
    {"not", Token::Type::NOT},
    {"print", Token::Type::PRINT},
    {"return", Token::Type::RETURN},
    {"break", Token::Type::BREAK},
    {"continue", Token::Type::CONTINUE},
    {"while", Token::Type::WHILE},
    {"for", Token::Type::FOR},
    {"super", Token::Type::SUPER},
    {"class", Token::Type::CLASS}
};

Token::Type Token::getKeyword(std::string keyword){
    return Keywords.at(keyword);
}

bool Token::isKeyword(std::string keyword){
    auto it = Keywords.find(keyword);
    return it != Keywords.end();
}
