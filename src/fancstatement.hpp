#pragma once

#include "token.hpp"
#include "fancexpr.hpp"
#include "interpreter.hpp"
#include <vector>

enum StmtIdentity {
    NONEXISTENT,
    VALID,
    ST_CONTINUE,
    ST_BREAK
};

class FanCStatement {

public:
    StmtIdentity identity = NONEXISTENT;

    virtual void accept(Interpreter interpreter);
    virtual ~FanCStatement() = default;

    FanCStatement(StmtIdentity identity) : identity(identity) {}
};


class BreakStmt : public FanCStatement {

void accept(Interpreter interpreter) { return; } // Does nothing.

public:
    BreakStmt() : FanCStatement(ST_BREAK) {};
};

class ContinueStmt : public FanCStatement {

void accept(Interpreter interpreter) { return; } // Does nothing.

public:
    ContinueStmt() : FanCStatement(ST_CONTINUE) {};
};


class BlockStmt : public FanCStatement {

void accept(Interpreter interpreter);

public:
    std::vector<FanCStatement> statements;

    BlockStmt(std::vector<FanCStatement> statements = {}, StmtIdentity identity = VALID) :
        FanCStatement(identity), statements(statements) {};
};

class WhileStmt : public FanCStatement {

void accept(Interpreter interpreter);

public:
    FanCExpr condition;
    BlockStmt body;

    WhileStmt(FanCExpr condition, BlockStmt body, StmtIdentity identity = VALID) :
        FanCStatement(identity), condition(condition), body(body) {};
};

class IfStmt : public FanCStatement {

void accept(Interpreter interpreter);

public:
    std::vector<FanCExpr> ifConditions;
    std::vector<BlockStmt> ifBranches;
    BlockStmt elseBranch;

    IfStmt(std::vector<FanCExpr> ifConditions, std::vector<BlockStmt> ifBranches,
        BlockStmt elseBranch, StmtIdentity identity = VALID) :

        FanCStatement(identity),
        ifConditions(ifConditions), ifBranches(ifBranches), elseBranch(elseBranch) {};

};

class VariableStmt : public FanCStatement {

void accept(Interpreter interpreter);

public:
    Token type;
    Token name;
    FanCExpr initializer;

    VariableStmt(Token type, Token name, FanCExpr initializer, StmtIdentity identity = VALID) :
        FanCStatement(identity), type(type), name(name), initializer(initializer) {}
};

class PrintStmt : public FanCStatement {

void accept(Interpreter interpreter);

public:
    FanCExpr expr;

    PrintStmt(FanCExpr expr, StmtIdentity identity = VALID) : FanCStatement(identity), expr(expr) {};
};

class ExprStmt : public FanCStatement {

void accept(Interpreter interpreter);

public:
    FanCExpr expr;

    ExprStmt(FanCExpr expr, StmtIdentity identity = VALID) : FanCStatement(identity), expr(expr) {};
};
