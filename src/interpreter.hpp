#pragma once

#include "fancerror.hpp"
#include "datatype.hpp"
#include "fancscope.hpp"
#include <godot_cpp/classes/ref_counted.hpp>
#include <string>
#include <variant>
#include <vector>

class FanCStatement;
class ForStmt;
class WhileStmt;
class IfStmt;
class BlockStmt;
class PrintStmt;
class VariableStmt;
class ExprStmt;
class FanCExpr;
class Assign;
class Variable;
class Literal;
class Grouping;
class Unary;
class Binary;
class Ternary;

class Interpreter {

public:
    static std::vector<FanCError> errors;

    Interpreter(){}

    void interpret(std::vector<FanCStatement> statements);

    void visitForStmt(ForStmt statement);

    void visitWhileStmt(WhileStmt statement);

    void visitIfStmt(IfStmt statement);

    void visitBlock(BlockStmt block);

    void visitExprStmt(ExprStmt statement);

    void visitPrintStmt(PrintStmt statement);

    void visitVarStmt(VariableStmt statement);

    std::variant<int, bool, double, std::string> visitAssign(const Assign &expr);

    std::variant<int, bool, double, std::string> visitVar(const Variable &expr);

    std::variant<int, bool, double, std::string> visitGrouping(const Grouping &expr);

    std::variant<int, bool, double, std::string> visitUnary(const Unary &expr);

    std::variant<int, bool, double, std::string> visitBinary(const Binary &expr);

    std::variant<int, bool, double, std::string> visitTernary(const Ternary &expr);

private:
    bool breaking = false;
    bool continuing = false;
    int loopCount = 0;
    FanCScope currentScope = FanCScope();

    void execute(FanCStatement statement);

    void executeBlock(std::vector<FanCStatement> &statements, FanCScope scope);

    std::variant<int, bool, double, std::string> evaluate(const FanCExpr &expr);

    std::string stringify(const std::variant<int, bool, double, std::string> &value);

    // Only use when you know for sure.
    double toFloat(const std::variant<int, bool, double, std::string> &value);

    int doubleToInt(const std::variant<int, bool, double, std::string> &value);

    // Only use when you know for sure.
    int toInt(const std::variant<int, bool, double, std::string> &value);

    bool eitherIsFloat(
        const std::variant<int, bool, double, std::string> &left,
        const std::variant<int, bool, double, std::string> &right
    );

    bool checkNumberOperands(
        const Token &op,
        const std::variant<int, bool, double, std::string> &left,
        const std::variant<int, bool, double, std::string> &right
    );

    bool checkNumberOperand(
        const Token &op,
        const std::variant<int, bool, double, std::string> &value
    );

    bool areEqual(
        const std::variant<int, bool, double, std::string> &left,
        const std::variant<int, bool, double, std::string> &right
    )
    {return isTruthy(left) == isTruthy(right);}

    bool isTruthy(const std::variant<int, bool, double, std::string> &value);

    DataType findType(const std::variant<int, bool, double, std::string> &value);

    void addComparisonError(const Token &op){
        errors.push_back(
                FanCError(op, FanCError::RUNTIME,
                "Operands must be two numbers!")
        );
    }
};
