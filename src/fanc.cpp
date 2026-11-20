#include "fanc.hpp"
#include "fancerror.hpp"
#include "fanclexer.hpp"
#include "fancparser.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "interpreter.hpp"
#include <string>
#include <sstream>

std::string FanC::source = "";

FanCLexer FanC::lexer = FanCLexer();
FanCParser FanC::parser = FanCParser();
Interpreter FanC::interpreter = Interpreter();

void FanC::_bind_methods(){
    godot::ClassDB::bind_method(godot::D_METHOD("run", "source_code"), &FanC::run);
}

void FanC::run(godot::String sourceCode){

    source = sourceCode.utf8().get_data();

    lexer.compileTokens(source);

    if (pushErrors(lexer.errors)) {
        ERR_PRINT("No longer running...");
        return;
    }

    lexer.printTokens();

    parser.parse(lexer.tokens);

    if (pushErrors(parser.errors)) {
        ERR_PRINT("No longer running...");
        return;
    }

    interpreter.interpret(parser.statements);

    if (pushErrors(interpreter.errors)) {
        ERR_PRINT("No longer running...");
        return;
    }

}

bool FanC::pushErrors(const std::vector<FanCError> &errors){

    if (errors.empty()) {return false;}

    bool stopRunning = false;

    for (FanCError error : errors){

        std::string msg = "Error at [" + std::to_string(error.lineNum) + ", " + std::to_string(error.column) + "]: ";

        msg += error.message;

        int arrowStart = error.column - error.length;
        std::string location = lpad("", arrowStart);
        location += repeat(error.length);
        location += "--- HERE";

        std::vector<std::string> sourceLines = split(source, '\n');

        ERR_PRINT(msg.c_str());
        ERR_PRINT(sourceLines[error.lineNum - 1].c_str());
        ERR_PRINT(location.c_str());

        if (error.type != FanCError::IGNORABLE) {
            stopRunning = true;
        }
    }

    return stopRunning;
}

std::string FanC::lpad(std::string str, size_t width) {
    if (str.length() >= width) {
        return str;
    }
    return std::string(width - str.length(), ' ') + str;
}

std::string FanC::repeat(int times){
    std::string n = "";
    for (int i = 0; i < times; i++){
        n += "^";
    }
    return n;
}

std::vector<std::string> FanC::split(const std::string& str, char delimiter) {
    std::vector<std::string> lines;
    std::stringstream ss(str);
    std::string line;

    while (std::getline(ss, line, delimiter)) {
        lines.push_back(line);
    }
    return lines;
}
