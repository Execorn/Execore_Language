#pragma once

#include "execore/ast/visitor.hpp"
#include "execore/ast/statements.hpp"
#include "execore/ast/expressions.hpp"
#include "execore/runtime/value.hpp"
#include "execore/runtime/environment.hpp"
#include "execore/runtime/builtins.hpp"
#include "execore/frontend/source_manager.hpp"
#include "execore/diagnostics/diagnostic_engine.hpp"
#include <iostream>
#include <memory>
#include <unordered_set>

namespace execore {

struct ReturnSignal {
    Value value;
};

struct BreakSignal {};
struct ContinueSignal {};

class Interpreter : public ASTVisitor {
public:
    Interpreter(DiagnosticEngine& diag, SourceManager* sm = nullptr,
                std::istream& in = std::cin, std::ostream& out = std::cout);
    ~Interpreter() override;

    int execute(Program& program);

    // Expressions
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

    // Statements
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

    [[nodiscard]] const std::shared_ptr<Environment>& environment() const noexcept {
        return current_env_;
    }

private:
    Value evaluate(Expr& expr);
    void execute_statement(Stmt& stmt);
    void execute_block(const BlockStmt& block, std::shared_ptr<Environment> env);
    std::shared_ptr<Environment> create_environment(std::shared_ptr<Environment> parent = nullptr);

    DiagnosticEngine& diag_;
    SourceManager* sm_{nullptr};
    std::istream& in_;
    std::ostream& out_;

    std::shared_ptr<Environment> global_env_;
    std::shared_ptr<Environment> current_env_;
    BuiltinRegistry builtins_;

    Value last_evaluated_value_{};
    std::unordered_set<std::string> loaded_modules_;
    std::vector<std::weak_ptr<Environment>> all_environments_;
};

} // namespace execore
