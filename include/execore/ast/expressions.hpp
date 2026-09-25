#pragma once

#include "execore/ast/ast_node.hpp"
#include "execore/common/types.hpp"
#include <string>
#include <vector>
#include <memory>

namespace execore {

enum class UnaryOp : uint8_t {
    Not,
    Minus,
    Plus
};

[[nodiscard]] std::string_view to_string(UnaryOp op) noexcept;

enum class BinaryOp : uint8_t {
    Add,
    Sub,
    Mul,
    Div,
    Mod,
    And,
    Or,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Equal,
    NotEqual,
    In
};

[[nodiscard]] std::string_view to_string(BinaryOp op) noexcept;

enum class AssignOp : uint8_t {
    Assign,
    AddAssign,
    SubAssign,
    MulAssign,
    DivAssign,
    ModAssign
};

[[nodiscard]] std::string_view to_string(AssignOp op) noexcept;

class LiteralExpr : public Expr {
public:
    LiteralExpr(TypeKind type, std::string value, SourceSpan span = {})
        : Expr(std::move(span)), type_(type), value_(std::move(value)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] TypeKind literal_type() const noexcept { return type_; }
    [[nodiscard]] const std::string& value() const noexcept { return value_; }

private:
    TypeKind type_;
    std::string value_;
};

class IdentifierExpr : public Expr {
public:
    IdentifierExpr(std::string name, SourceSpan span = {})
        : Expr(std::move(span)), name_(std::move(name)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] const std::string& name() const noexcept { return name_; }

private:
    std::string name_;
};

class UnaryExpr : public Expr {
public:
    UnaryExpr(UnaryOp op, std::unique_ptr<Expr> operand, SourceSpan span = {})
        : Expr(std::move(span)), op_(op), operand_(std::move(operand)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] UnaryOp op() const noexcept { return op_; }
    [[nodiscard]] Expr* operand() const noexcept { return operand_.get(); }
    [[nodiscard]] std::unique_ptr<Expr> take_operand() noexcept { return std::move(operand_); }

private:
    UnaryOp op_;
    std::unique_ptr<Expr> operand_;
};

class BinaryExpr : public Expr {
public:
    BinaryExpr(BinaryOp op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right, SourceSpan span = {})
        : Expr(std::move(span)), op_(op), left_(std::move(left)), right_(std::move(right)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] BinaryOp op() const noexcept { return op_; }
    [[nodiscard]] Expr* left() const noexcept { return left_.get(); }
    [[nodiscard]] Expr* right() const noexcept { return right_.get(); }

private:
    BinaryOp op_;
    std::unique_ptr<Expr> left_;
    std::unique_ptr<Expr> right_;
};

class AssignExpr : public Expr {
public:
    AssignExpr(AssignOp op, std::unique_ptr<Expr> target, std::unique_ptr<Expr> value, SourceSpan span = {})
        : Expr(std::move(span)), op_(op), target_(std::move(target)), value_(std::move(value)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] AssignOp op() const noexcept { return op_; }
    [[nodiscard]] Expr* target() const noexcept { return target_.get(); }
    [[nodiscard]] Expr* value() const noexcept { return value_.get(); }

private:
    AssignOp op_;
    std::unique_ptr<Expr> target_;
    std::unique_ptr<Expr> value_;
};

class CallExpr : public Expr {
public:
    CallExpr(std::string callee, std::vector<std::unique_ptr<Expr>> args, SourceSpan span = {})
        : Expr(std::move(span)), callee_(std::move(callee)), args_(std::move(args)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] const std::string& callee() const noexcept { return callee_; }
    [[nodiscard]] const std::vector<std::unique_ptr<Expr>>& args() const noexcept { return args_; }

private:
    std::string callee_;
    std::vector<std::unique_ptr<Expr>> args_;
};

class IndexExpr : public Expr {
public:
    IndexExpr(std::unique_ptr<Expr> sequence, std::unique_ptr<Expr> index, SourceSpan span = {})
        : Expr(std::move(span)), sequence_(std::move(sequence)), index_(std::move(index)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] Expr* sequence() const noexcept { return sequence_.get(); }
    [[nodiscard]] Expr* index() const noexcept { return index_.get(); }

private:
    std::unique_ptr<Expr> sequence_;
    std::unique_ptr<Expr> index_;
};

class SliceExpr : public Expr {
public:
    SliceExpr(std::unique_ptr<Expr> sequence,
              std::unique_ptr<Expr> start,
              std::unique_ptr<Expr> end,
              SourceSpan span = {})
        : Expr(std::move(span)), sequence_(std::move(sequence)),
          start_(std::move(start)), end_(std::move(end)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] Expr* sequence() const noexcept { return sequence_.get(); }
    [[nodiscard]] Expr* start() const noexcept { return start_.get(); }
    [[nodiscard]] Expr* end() const noexcept { return end_.get(); }

private:
    std::unique_ptr<Expr> sequence_;
    std::unique_ptr<Expr> start_;
    std::unique_ptr<Expr> end_;
};

class MethodCallExpr : public Expr {
public:
    MethodCallExpr(std::unique_ptr<Expr> object, std::string method_name,
                   std::vector<std::unique_ptr<Expr>> args, SourceSpan span = {})
        : Expr(std::move(span)), object_(std::move(object)),
          method_name_(std::move(method_name)), args_(std::move(args)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] Expr* object() const noexcept { return object_.get(); }
    [[nodiscard]] const std::string& method_name() const noexcept { return method_name_; }
    [[nodiscard]] const std::vector<std::unique_ptr<Expr>>& args() const noexcept { return args_; }

private:
    std::unique_ptr<Expr> object_;
    std::string method_name_;
    std::vector<std::unique_ptr<Expr>> args_;
};

class CommaExpr : public Expr {
public:
    explicit CommaExpr(std::vector<std::unique_ptr<Expr>> expressions, SourceSpan span = {})
        : Expr(std::move(span)), expressions_(std::move(expressions)) {}

    void accept(ASTVisitor& visitor) override { visitor.visit(*this); }

    [[nodiscard]] const std::vector<std::unique_ptr<Expr>>& expressions() const noexcept {
        return expressions_;
    }

private:
    std::vector<std::unique_ptr<Expr>> expressions_;
};

} // namespace execore
