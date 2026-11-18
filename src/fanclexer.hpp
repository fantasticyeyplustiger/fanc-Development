#pragma once

#include "fancerror.hpp"
#include "token.hpp"
#include <godot_cpp/classes/ref_counted.hpp>
#include <vector>

class FanCLexer {

public:
    std::vector<FanCError> errors;
    std::vector<Token> compileTokens(const std::string& sourceCode);
    FanCLexer();

private:

    int start = 0;
    int current = 0;
    int lineNum = 0;
    int column = 0;
    int end = 0;
    std::string& sourceCode;
    std::vector<Token> tokens;

    void printTokens();

    void scanToken();

    void reportUnexpectedChar();

    bool nextIs(const char expected);

    bool isDigit(const char ch);

    bool isAlpha(const char ch);

    bool isAlphaDigit(const char ch);

    bool isDecimal();

    bool isScientificNotation();

    bool isEnd();

    void identifierToken();

    void numberToken();

    void stringToken();

    void skipCommentRegion();

    void skipCommentLine();

    char peek(int extra = 0);

    char advance();

    void addToken(Type type, std::variant<int, bool, double, std::string> literal = false);

    };
