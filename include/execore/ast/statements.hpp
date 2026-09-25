#pragma once

#include "execore/ast/ast_node.hpp"
#include "execore/ast/expressions.hpp"
#include "execore/common/types.hpp"
#include <string>
#include <vector>
#include <memory>

namespace execore {

class ExprStmt : public Stmt {
public:
    explicit ExprStmt(std::unique_ptr<Expr> expr, SourceSpan span = {})
        : Stmt(std::move(span)), expr_(std::move(expr)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] Expr* expr() const noexcept { return expr_.get(); }

private:
    std::unique_ptr<Expr> expr_;
};

class BlockStmt : public Stmt {
public:
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements = {}, SourceSpan span = {})
        : Stmt(std::move(span)), statements_(std::move(statements)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    void add_statement(std::unique_ptr<Stmt> stmt) {
        statements_.push_back(std::move(stmt));
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Stmt>>& statements() const noexcept {
        return statements_;
    }

private:
    std::vector<std::unique_ptr<Stmt>> statements_;
};

struct VarInit {
    std::string name;
    std::unique_ptr<Expr> init_value; // can be nullptr
    SourceSpan span;
};

class VarDeclStmt : public Stmt {
public:
    VarDeclStmt(TypeKind type, std::vector<VarInit> variables, SourceSpan span = {})
        : Stmt(std::move(span)), type_(type), variables_(std::move(variables)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] TypeKind type() const noexcept { return type_; }
    [[nodiscard]] const std::vector<VarInit>& variables() const noexcept { return variables_; }

private:
    TypeKind type_;
    std::vector<VarInit> variables_;
};

class FunctionDeclStmt : public Stmt {
public:
    FunctionDeclStmt(std::string name, std::vector<std::string> params,
                     std::unique_ptr<BlockStmt> body, SourceSpan span = {})
        : Stmt(std::move(span)), name_(std::move(name)),
          params_(std::move(params)), body_(std::move(body)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::vector<std::string>& params() const noexcept { return params_; }
    [[nodiscard]] BlockStmt* body() const noexcept { return body_.get(); }

private:
    std::string name_;
    std::vector<std::string> params_;
    std::unique_ptr<BlockStmt> body_;
};

class IfStmt : public Stmt {
public:
    IfStmt(std::unique_ptr<Expr> condition, std::unique_ptr<BlockStmt> then_branch,
           std::unique_ptr<BlockStmt> else_branch = nullptr, SourceSpan span = {})
        : Stmt(std::move(span)), condition_(std::move(condition)),
          then_branch_(std::move(then_branch)), else_branch_(std::move(else_branch)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] Expr* condition() const noexcept { return condition_.get(); }
    [[nodiscard]] BlockStmt* then_branch() const noexcept { return then_branch_.get(); }
    [[nodiscard]] BlockStmt* else_branch() const noexcept { return else_branch_.get(); }

private:
    std::unique_ptr<Expr> condition_;
    std::unique_ptr<BlockStmt> then_branch_;
    std::unique_ptr<BlockStmt> else_branch_;
};

class WhileStmt : public Stmt {
public:
    WhileStmt(std::unique_ptr<Expr> condition, std::unique_ptr<BlockStmt> body, SourceSpan span = {})
        : Stmt(std::move(span)), condition_(std::move(condition)), body_(std::move(body)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] Expr* condition() const noexcept { return condition_.get(); }
    [[nodiscard]] BlockStmt* body() const noexcept { return body_.get(); }

private:
    std::unique_ptr<Expr> condition_;
    std::unique_ptr<BlockStmt> body_;
};

class DoWhileStmt : public Stmt {
public:
    DoWhileStmt(std::unique_ptr<BlockStmt> body, std::unique_ptr<Expr> condition, SourceSpan span = {})
        : Stmt(std::move(span)), body_(std::move(body)), condition_(std::move(condition)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] BlockStmt* body() const noexcept { return body_.get(); }
    [[nodiscard]] Expr* condition() const noexcept { return condition_.get(); }

private:
    std::unique_ptr<BlockStmt> body_;
    std::unique_ptr<Expr> condition_;
};

class ForStmt : public Stmt {
public:
    ForStmt(std::string var_name, std::unique_ptr<Expr> sequence,
            std::unique_ptr<BlockStmt> body, SourceSpan span = {})
        : Stmt(std::move(span)), var_name_(std::move(var_name)),
          sequence_(std::move(sequence)), body_(std::move(body)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] const std::string& var_name() const noexcept { return var_name_; }
    [[nodiscard]] Expr* sequence() const noexcept { return sequence_.get(); }
    [[nodiscard]] BlockStmt* body() const noexcept { return body_.get(); }

private:
    std::string var_name_;
    std::unique_ptr<Expr> sequence_;
    std::unique_ptr<BlockStmt> body_;
};

class ReturnStmt : public Stmt {
public:
    explicit ReturnStmt(std::unique_ptr<Expr> value = nullptr, SourceSpan span = {})
        : Stmt(std::move(span)), value_(std::move(value)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] Expr* value() const noexcept { return value_.get(); }

private:
    std::unique_ptr<Expr> value_;
};

class BreakStmt : public Stmt {
public:
    explicit BreakStmt(SourceSpan span = {}) : Stmt(std::move(span)) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class ContinueStmt : public Stmt {
public:
    explicit ContinueStmt(SourceSpan span = {}) : Stmt(std::move(span)) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class PassStmt : public Stmt {
public:
    explicit PassStmt(SourceSpan span = {}) : Stmt(std::move(span)) {}
    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }
};

class PrintStmt : public Stmt {
public:
    PrintStmt(bool raw, std::vector<std::unique_ptr<Expr>> expressions, SourceSpan span = {})
        : Stmt(std::move(span)), raw_(raw), expressions_(std::move(expressions)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] bool is_raw() const noexcept { return raw_; }
    [[nodiscard]] const std::vector<std::unique_ptr<Expr>>& expressions() const noexcept {
        return expressions_;
    }

private:
    bool raw_{false};
    std::vector<std::unique_ptr<Expr>> expressions_;
};

struct InputItem {
    std::string prompt; // optional, can be empty
    std::string var_name;
    SourceSpan span;
};

class InputStmt : public Stmt {
public:
    explicit InputStmt(std::vector<InputItem> items, SourceSpan span = {})
        : Stmt(std::move(span)), items_(std::move(items)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] const std::vector<InputItem>& items() const noexcept { return items_; }

private:
    std::vector<InputItem> items_;
};

class ImportStmt : public Stmt {
public:
    explicit ImportStmt(std::vector<std::unique_ptr<Expr>> modules, SourceSpan span = {})
        : Stmt(std::move(span)), modules_(std::move(modules)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] const std::vector<std::unique_ptr<Expr>>& modules() const noexcept {
        return modules_;
    }

private:
    std::vector<std::unique_ptr<Expr>> modules_;
};

class Program : public ASTNode {
public:
    explicit Program(std::vector<std::unique_ptr<Stmt>> statements = {}, SourceSpan span = {})
        : ASTNode(std::move(span)), statements_(std::move(statements)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    void add_statement(std::unique_ptr<Stmt> stmt) {
        statements_.push_back(std::move(stmt));
    }

    [[nodiscard]] const std::vector<std::unique_ptr<Stmt>>& statements() const noexcept {
        return statements_;
    }

private:
    std::vector<std::unique_ptr<Stmt>> statements_;
};

} // namespace execore
