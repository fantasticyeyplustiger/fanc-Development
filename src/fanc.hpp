#pragma once

#include "fanclexer.hpp"
#include "fancparser.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "interpreter.hpp"
#include <godot_cpp/classes/object.hpp>
#include <vector>

class FanC : public godot::Object {

GDCLASS(FanC, godot::Object)

protected:
    static void _bind_methods();

private:

    static FanCLexer lexer;
    static FanCParser parser;
    static Interpreter interpreter;

    static std::string source;

    void run(godot::String sourceCode);

    static bool pushErrors(const std::vector<FanCError> &errors);

    static std::string lpad(std::string str, size_t width);

    static std::string repeat(int times);

    static std::vector<std::string> split(const std::string& str, char delimiter);
};

