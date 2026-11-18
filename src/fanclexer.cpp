#include "Token.hpp"
#include "fancerror.hpp"
#include "token.hpp"
#include "fanclexer.hpp"
#include <vector>
#include <string>

void FanCLexer::printTokens(){
    for (Token token : tokens) {
        godot::print_line(godot::String(token.toString().c_str()));
    }
}

std::vector<Token> FanCLexer::compileTokens(const std::string& sourceCode){
    this -> sourceCode = sourceCode;

    start = 0;
    current = 0;
    lineNum = 1;
    column = -1;
    end = sourceCode.length();
    tokens = {};

    while (!isEnd()){
        scanToken();
        start = current;
    }

    addToken(Type::END);
    return tokens;
};

void FanCLexer::scanToken() {
    char ch = advance();

    switch (ch) {

        case '(': addToken(Type::LEFT_PAREN); break;
        case ')': addToken(Type::RIGHT_PAREN); break;
        case '[': addToken(Type::LEFT_BRACKET); break;
        case ']': addToken(Type::RIGHT_BRACKET); break;
        case '{': addToken(Type::LEFT_BRACE); break;
        case '}': addToken(Type::RIGHT_BRACE); break;

        case '?': addToken(Type::TERNARY); break;
        case ':': addToken(Type::COLON); break;
        case ';': addToken(Type::SEMICOLON); break;
        case ',': addToken(Type::COMMA); break;
        case '.': addToken(Type::DOT); break;

        case '+': addToken(nextIs('=') ? Type::PLUS_EQUAL : Type::PLUS); break;
        case '-': addToken(nextIs('=') ? Type::MINUS_EQUAL : Type::MINUS); break;
        case '*': addToken(nextIs('=') ? Type::STAR_EQUAL : Type::STAR); break;
        case '%': addToken(nextIs('=') ? Type::MODULO_EQUAL : Type::MODULO); break;
        case '/': {
            if (nextIs('*')) { skipCommentRegion(); }
            else if (nextIs('/')) { skipCommentLine(); }

            else{
                addToken(nextIs('=') ? Type::SLASH_EQUAL : Type::SLASH);
            }

            break;
        }

        case '!': addToken(nextIs('=') ? Type::NOT_EQUAL : Type::NOT); break;
        case '=': addToken(nextIs('=') ? Type::EQUAL_EQUAL : Type::EQUAL); break;
        case '<': addToken(nextIs('=') ? Type::LESS_EQUAL : Type::LESS); break;
        case '>': addToken(nextIs('=') ? Type::GREATER_EQUAL : Type::GREATER); break;

        case '&': {
            if (nextIs('&')) { addToken(Type::AND); }
            else { reportUnexpectedChar(); }
            break;
        }

        case '|': {
            if (nextIs('|')) { addToken(Type::OR); }
            else { reportUnexpectedChar(); }
            break;
        }
        
        case '#': skipCommentLine(); break;
        
        case '"': stringToken(); break;

        case ' ': break;
        case '\r': column = 0; break;
        case '\t': column += 4; break;
        case '\n': lineNum++; column = 0; break;

        default: {

            if (isDigit(ch)) { numberToken(); }
            else if (isAlpha(ch)) { identifierToken(); }
            else { reportUnexpectedChar(); }

        }

    }
}

void FanCLexer::reportUnexpectedChar(){
    errors.push_back(FanCError(lineNum, column, 1, FanCError::LEXER, "Unexpected character!"));
}

char FanCLexer::peek(int extra){
    if (current + extra >= end) {return '\0';}
    return sourceCode[current + extra];
};

char FanCLexer::advance(){
    char ch = sourceCode[current];
    current++;
    column++;
    return ch;
}

bool FanCLexer::nextIs(const char expected){
    if (isEnd()) {return false;}
    if (sourceCode[current] != expected) {return false;}

    current++;
    column++;
    return true;
}

bool FanCLexer::isDigit(const char ch){
    return ch >= '0' and ch <= '9';
}

bool FanCLexer::isAlpha(const char ch){
    return ch != ' ' && (ch >= 'a' and ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_';
}

bool FanCLexer::isAlphaDigit(const char ch){
    return isDigit(ch) || isAlpha(ch);
}

bool FanCLexer::isScientificNotation(){
    return (peek() == 'e' || peek() == 'E') && isDigit(peek(1));
}

bool FanCLexer::isDecimal(){
    return (peek() == '.') && isDigit(peek(1));
}

bool FanCLexer::isEnd(){ return current >= end; };

void FanCLexer::stringToken(){

    while (peek() != '"' && !isEnd()){

        if (peek() == '\n') {
            lineNum++;
            column = 0;
        }

        advance();

    }

    // If this is true then last " wasn't found
    if (isEnd()){
        errors.push_back(FanCError(lineNum, column, 1, FanCError::COMPILER, "Unterminated string!"));
        return;
    }

    // Consume the closing "
    advance();

    std::string value = sourceCode.substr(start + 1, (current - (start + 2)));
    addToken(Type::STRING, value);
}

void FanCLexer::numberToken(){

    std::string numberText;
    std::string exponentText;

    // When numberToken is called Lexer must have detected a single digit number
    int numberTextLength = 1;
    int exponentLength = 0;

    bool isInt = true;

    if (isScientificNotation()){

        isInt = false; // Scientific notation turns numbers into floats
        
        advance(); // Consume e

        while (isDigit(peek())){
            exponentLength++;
            advance();
        }
    }

    else if (isDecimal()){

        // Includes the decimal point
        numberTextLength++;
        isInt = false;
        advance();

        while (isDigit(peek())){
            advance(); // Consume .

            if (isScientificNotation()){
                advance(); // Consume e

                while (isDigit(peek())){
                    exponentLength++;
                    advance();
                }
            }
        }
    }

    else { // Horrible nesting lol

        while (isDigit(peek())){
            numberTextLength++;
            advance();

            if (isDecimal()){
                numberTextLength++;
                advance(); // Consume .
                isInt = false;

                while (isDigit(peek())){
                    numberTextLength++;
                    advance();

                    if (isScientificNotation()){
                        advance(); // Consume e

                        while (isDigit(peek())){
                            exponentLength++;
                            advance();
                        }
                    }
                }
            }
        }

    }

    numberText = sourceCode.substr(start, numberTextLength);

    if (exponentLength > 0){
        exponentText = sourceCode.substr(start + numberTextLength, exponentLength);
    }

    if (isInt){
        int number = std::stoi(numberText);
        addToken(Type::NUMBER, number);
    }
    else{

        if (exponentLength > 0){

            int exponent = std::stoi(exponentText);
            float number = std::stof(numberText);

            addToken(Type::NUMBER, number * std::pow(10, exponent));

        }
        else{
            float number = std::stof(numberText);
            addToken(Type::NUMBER, number);
        }

    }

}

void FanCLexer::identifierToken(){
    while (isAlphaDigit(peek())) { advance(); }

    std::string text = sourceCode.substr(start, (current - start));

    if (Token::isKeyword(text)){
        addToken(Token::getKeyword(text));
    }
    else {
        addToken(Type::IDENTIFIER);
    }
}

void FanCLexer::skipCommentRegion(){

    while (peek() != '*' && peek(1) != '/' && !isEnd()){
        if (advance() == '\n'){
            lineNum++;
            column = 0;
        }
    }

    // Consumes the two characters
    if (peek() != '*' && peek(1) != '/'){
        advance();
        advance();
    }
}

void FanCLexer::skipCommentLine(){
    while (peek() != '\n' && !isEnd()) {advance();}
}

void FanCLexer::addToken(Type type, std::variant<int, bool, double, std::string> literal){
    std::string text = sourceCode.substr(start, current - start);
    tokens.push_back(Token(lineNum, column, type, text, literal));
}
