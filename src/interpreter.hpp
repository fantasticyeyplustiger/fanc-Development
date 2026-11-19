#pragma once

#include "fancerror.hpp"
#include "fancscope.hpp"
#include "fancstatement.hpp"
#include <string>
#include <variant>
#include <vector>

enum DataType {
    DT_INT = 0,
    DT_BOOL = 1,
    DT_FLOAT = 2,
    DT_STR = 3,
    OTHER = 4
};

class Interpreter {

public:
    std::vector<FanCError> errors = {};

private:
    bool breaking = false;
    bool continuing = false;
    int loopCount = 0;
    // scope = FanCScope

    void interpret(std::vector<FanCStatement> statements);

    void execute(FanCStatement statement) { statement.accept(this); }

    void executeBlock(std::vector<FanCStatement> &statements, FanCScope scope);

    // void visitForStmt()

    void visitWhileStmt(WhileStmt statement);

    void visitIfStmt(IfStmt statement);

    void visitBlock(BlockStmt block);

    void visitExprStmt(ExprStmt statement);

    void visitPrintStmt(ExprStmt statement);

    void visitVarStmt(ExprStmt statement);

    std::variant<int, bool, double, std::string> evaluate(FanCExpr expr) { return expr.accept(this); }

    std::variant<int, bool, double, std::string> visitAssign(Assign expr);

    std::variant<int, bool, double, std::string> visitVar(Variable expr);

    std::variant<int, bool, double, std::string> visitGrouping(Grouping expr);

    std::variant<int, bool, double, std::string> visitUnary(Unary expr);

    std::variant<int, bool, double, std::string> visitBinary(Binary expr);

    std::variant<int, bool, double, std::string> visitTernary(Ternary expr);

    std::string stringify(std::variant<int, bool, double, std::string> value);

    // Only use when you know for sure.
    double toFloat(std::variant<int, bool, double, std::string> value);

    int doubleToInt(std::variant<int, bool, double, std::string> value);

    // Only use when you know for sure.
    int toInt(std::variant<int, bool, double, std::string> value);

    bool eitherIsFloat(
        std::variant<int, bool, double, std::string> left,
        std::variant<int, bool, double, std::string> right
    );

    bool checkNumberOperands(
        Token op,
        std::variant<int, bool, double, std::string> left,
        std::variant<int, bool, double, std::string> right
    );

    bool checkNumberOperand(
        Token op,
        std::variant<int, bool, double, std::string> value
    );

    bool areEqual(
        std::variant<int, bool, double, std::string> left,
        std::variant<int, bool, double, std::string> right
    )
    {return isTruthy(left) == isTruthy(right);}

    bool isTruthy(std::variant<int, bool, double, std::string> value);

    DataType findType(std::variant<int, bool, double, std::string> value);

    void addComparisonError(Token op){
        errors.push_back(
                FanCError(op, FanCError::RUNTIME,
                "Operands must be two numbers!")
        );
    }
};
