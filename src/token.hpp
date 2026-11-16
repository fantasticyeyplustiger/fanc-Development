#pragma once

#include "godot_cpp/variant/string.hpp"
#include <charconv>
#include <string>
#include <variant>

class Token {

enum Type {
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
	LEFT_BRACKET, RIGHT_BRACKET, COMMA, DOT, SEMICOLON,
	COMMENT,
	
	// One-Two Character Tokens
	NOT, NOT_EQUAL, TERNARY, COLON,
	AND, OR,
	EQUAL, EQUAL_EQUAL,
	GREATER, GREATER_EQUAL,
	LESS, LESS_EQUAL,
	MINUS, PLUS, SLASH, STAR, MODULO,
	MINUS_EQUAL, PLUS_EQUAL, SLASH_EQUAL,
	STAR_EQUAL, MODULO_EQUAL,
	PLUS_PLUS, MINUS_MINUS,
	
	// Literals
	IDENTIFIER, STRING, NUMBER,
	
	// Keywords
	CLASS, TRUE, FALSE, IF, ELIF, ELSE,
	PRINT, RETURN, SUPER, THIS,
	VAR, INT, FLOAT, BOOL, STR, // Data types
	WHILE, FOR, BREAK, CONTINUE,
	END
};

public:

int lineNum;
int column;
Type type;
std::string lexeme;
std::variant<int, bool, double, std::string> literal;

Token(int lineNum, int column, Type type, std::string lexeme, std::variant<int, bool, double, std::string> literal) :
    lineNum(lineNum), column(column), type(type), lexeme(lexeme), literal(literal) {
}

std::string toString();
    
};
