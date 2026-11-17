#include "godot_cpp/core/object.hpp"
#include "token.hpp"
#include "fanclexer.hpp"
#include <vector>
#include <string>

void FanCLexer::_bind_methods(){

    godot::ClassDB::bind_method(godot::D_METHOD("compile_tokens", "source_code"), &FanCLexer::compileTokens);
    godot::ClassDB::bind_method(godot::D_METHOD("print_tokens"), &FanCLexer::printTokens);
    
    ADD_SIGNAL(godot::MethodInfo("lexer_errored",
            godot::PropertyInfo(godot::Variant::INT, "line_number"),
            godot::PropertyInfo(godot::Variant::INT, "column"),
            godot::PropertyInfo(godot::Variant::INT, "lexeme_length"),
            godot::PropertyInfo(godot::Variant::STRING, "error_message")
    ));
    
};

void FanCLexer::printTokens(){
    for (Token token : tokens) {
        godot::print_line(godot::String(token.toString().c_str()));
    }
}

void FanCLexer::compileTokens(const godot::String sourceCode){
    this -> sourceCode = sourceCode.utf8().get_data();

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

    addToken(Token::Type::END);
};

void FanCLexer::scanToken() {
    char ch = advance();

    switch (ch) {

        case '(': addToken(Token::Type::LEFT_PAREN); break;
        case ')': addToken(Token::Type::RIGHT_PAREN); break;
        case '[': addToken(Token::Type::LEFT_BRACKET); break;
        case ']': addToken(Token::Type::RIGHT_BRACKET); break;
        case '{': addToken(Token::Type::LEFT_BRACE); break;
        case '}': addToken(Token::Type::RIGHT_BRACE); break;

        case '?': addToken(Token::Type::TERNARY); break;
        case ':': addToken(Token::Type::COLON); break;
        case ';': addToken(Token::Type::SEMICOLON); break;
        case ',': addToken(Token::Type::COMMA); break;
        case '.': addToken(Token::Type::DOT); break;

        case '+': addToken(nextIs('=') ? Token::Type::PLUS_EQUAL : Token::Type::PLUS); break;
        case '-': addToken(nextIs('=') ? Token::Type::MINUS_EQUAL : Token::Type::MINUS); break;
        case '*': addToken(nextIs('=') ? Token::Type::STAR_EQUAL : Token::Type::STAR); break;
        case '%': addToken(nextIs('=') ? Token::Type::MODULO_EQUAL : Token::Type::MODULO); break;
        case '/': {
            if (nextIs('*')) { skipCommentRegion(); }
            else if (nextIs('/')) { skipCommentLine(); }

            else{
                addToken(nextIs('=') ? Token::Type::SLASH_EQUAL : Token::Type::SLASH);
            }

            break;
        }

        case '!': addToken(nextIs('=') ? Token::Type::NOT_EQUAL : Token::Type::NOT); break;
        case '=': addToken(nextIs('=') ? Token::Type::EQUAL_EQUAL : Token::Type::EQUAL); break;
        case '<': addToken(nextIs('=') ? Token::Type::LESS_EQUAL : Token::Type::LESS); break;
        case '>': addToken(nextIs('=') ? Token::Type::GREATER_EQUAL : Token::Type::GREATER); break;

        case '&': {
            if (nextIs('&')) { addToken(Token::Type::AND); }
            else { reportUnexpectedChar(); }
            break;
        }

        case '|': {
            if (nextIs('|')) { addToken(Token::Type::OR); }
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
    emit_signal("lexer_errored", lineNum, column, 1, "Unexpected character!");
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
        emit_signal("lexer_errored", lineNum, column, 1, "Unterminated string!");
        return;
    }

    // Consume the closing "
    advance();

    std::string value = sourceCode.substr(start + 1, (current - (start + 2)));
    addToken(Token::Type::STRING, value);
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
        addToken(Token::Type::NUMBER, number);
    }
    else{

        if (exponentLength > 0){

            int exponent = std::stoi(exponentText);
            float number = std::stof(numberText);

            addToken(Token::Type::NUMBER, number * std::pow(10, exponent));

        }
        else{
            float number = std::stof(numberText);
            addToken(Token::Type::NUMBER, number);
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
        addToken(Token::Type::IDENTIFIER);
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

void FanCLexer::addToken(Token::Type type, std::variant<int, bool, double, std::string> literal){
    std::string text = sourceCode.substr(start, current - start);
    tokens.push_back(Token(lineNum, column, type, text, literal));
}
