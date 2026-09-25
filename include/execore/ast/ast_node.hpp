#pragma once

#include "execore/ast/ast_fwd.hpp"
#include "execore/ast/visitor.hpp"
#include "execore/common/source_location.hpp"
#include <memory>
#include <string>

namespace execore {

class ASTNode {
public:
    explicit ASTNode(SourceSpan span = {}) : span_(std::move(span)) {}
    virtual ~ASTNode() = default;

    virtual void accept(ASTVisitor& visitor) = 0;

    [[nodiscard]] const SourceSpan& span() const noexcept { return span_; }
    void set_span(SourceSpan span) noexcept { span_ = std::move(span); }

private:
    SourceSpan span_{};
};

class Expr : public ASTNode {
public:
    using ASTNode::ASTNode;
};

class Stmt : public ASTNode {
public:
    using ASTNode::ASTNode;
};

} // namespace execore
