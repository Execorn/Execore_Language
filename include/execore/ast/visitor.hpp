#pragma once

#include "execore/ast/ast_fwd.hpp"

namespace execore {

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    // Expressions
    virtual void visit(LiteralExpr& node) = 0;
    virtual void visit(IdentifierExpr& node) = 0;
    virtual void visit(UnaryExpr& node) = 0;
    virtual void visit(BinaryExpr& node) = 0;
    virtual void visit(AssignExpr& node) = 0;
    virtual void visit(CallExpr& node) = 0;
    virtual void visit(IndexExpr& node) = 0;
    virtual void visit(SliceExpr& node) = 0;
    virtual void visit(MethodCallExpr& node) = 0;
    virtual void visit(CommaExpr& node) = 0;

    // Statements
    virtual void visit(ExprStmt& node) = 0;
    virtual void visit(BlockStmt& node) = 0;
    virtual void visit(VarDeclStmt& node) = 0;
    virtual void visit(FunctionDeclStmt& node) = 0;
    virtual void visit(IfStmt& node) = 0;
    virtual void visit(WhileStmt& node) = 0;
    virtual void visit(DoWhileStmt& node) = 0;
    virtual void visit(ForStmt& node) = 0;
    virtual void visit(ReturnStmt& node) = 0;
    virtual void visit(BreakStmt& node) = 0;
    virtual void visit(ContinueStmt& node) = 0;
    virtual void visit(PassStmt& node) = 0;
    virtual void visit(PrintStmt& node) = 0;
    virtual void visit(InputStmt& node) = 0;
    virtual void visit(ImportStmt& node) = 0;
    virtual void visit(Program& node) = 0;
};

} // namespace execore
