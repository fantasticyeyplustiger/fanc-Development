#pragma once

#include <string>
#include <variant>
#include <unordered_map>

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

class Token {

public:

	static const std::unordered_map<std::string, Type> Keywords;

	int lineNum;
	int column;
	Type type;
	std::string lexeme;
	std::variant<int, bool, double, std::string> literal;

	Token(int lineNum, int column, Type type, std::string lexeme, std::variant<int, bool, double, std::string> literal = false) :
		lineNum(lineNum), column(column), type(type), lexeme(lexeme), literal(literal) {
	}

	static Type getKeyword(std::string keyword);

	static bool isKeyword(std::string keyword);

	std::string toString();
    
};
