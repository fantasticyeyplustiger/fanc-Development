#pragma once

#include "Token.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/string.hpp"
#include <godot_cpp/classes/ref_counted.hpp>
#include <vector>

class FanCLexer : public godot::RefCounted{

    GDCLASS(FanCLexer, godot::RefCounted)

public:
std::vector<Token> getTokens(const godot::String &sourceCode);

private:

int start = 0;
int current = 0;
int line_num = 0;
int column = 0;
int end = 0;
std::string sourceCode;
std::vector<Token> tokens;

void scanToken();

void reportUnexpectedChar();

bool nextIs(const char expected);

bool isDigit(const char ch);

bool isAlpha(const char ch);

bool isAlphaDigit(const char ch);

bool isEnd();

void identifierToken();

void numberToken();

void stringToken();

void skipCommentRegion();

void peek();

void advance();

void addToken();

};
