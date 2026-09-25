#pragma once

#include "execore/ast/visitor.hpp"
#include "execore/ast/statements.hpp"
#include "execore/ast/expressions.hpp"
#include "execore/semantics/symbol_table.hpp"
#include "execore/diagnostics/diagnostic_engine.hpp"

namespace execore {

class SemanticAnalyzer : public ASTVisitor {
public:
    SemanticAnalyzer(SymbolTable& symbols, DiagnosticEngine& diag);

    bool analyze(Program& program);

    void visit(LiteralExpr& node) override;
    void visit(IdentifierExpr& node) override;
    void visit(UnaryExpr& node) override;
    void visit(BinaryExpr& node) override;
    void visit(AssignExpr& node) override;
    void visit(CallExpr& node) override;
    void visit(IndexExpr& node) override;
    void visit(SliceExpr& node) override;
    void visit(MethodCallExpr& node) override;
    void visit(CommaExpr& node) override;

    void visit(ExprStmt& node) override;
    void visit(BlockStmt& node) override;
    void visit(VarDeclStmt& node) override;
    void visit(FunctionDeclStmt& node) override;
    void visit(IfStmt& node) override;
    void visit(WhileStmt& node) override;
    void visit(DoWhileStmt& node) override;
    void visit(ForStmt& node) override;
    void visit(ReturnStmt& node) override;
    void visit(BreakStmt& node) override;
    void visit(ContinueStmt& node) override;
    void visit(PassStmt& node) override;
    void visit(PrintStmt& node) override;
    void visit(InputStmt& node) override;
    void visit(ImportStmt& node) override;
    void visit(Program& node) override;

private:
    void register_builtins();

    SymbolTable& symbols_;
    DiagnosticEngine& diag_;
    size_t loop_depth_{0};
    size_t function_depth_{0};
};

} // namespace execore
