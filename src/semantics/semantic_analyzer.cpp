#include "execore/semantics/semantic_analyzer.hpp"

namespace execore {

SemanticAnalyzer::SemanticAnalyzer(SymbolTable& symbols, DiagnosticEngine& diag)
    : symbols_(symbols), diag_(diag) {
    register_builtins();
}

void SemanticAnalyzer::register_builtins() {
    symbols_.define_builtin(Symbol{"chr", SymbolKind::Function, TypeKind::Str, 1, {"int_val"}, {}});
    symbols_.define_builtin(Symbol{"ord", SymbolKind::Function, TypeKind::Int, 1, {"char_or_str"}, {}});
    symbols_.define_builtin(Symbol{"type", SymbolKind::Function, TypeKind::Str, 1, {"val"}, {}});
    symbols_.define_builtin(Symbol{"len", SymbolKind::Function, TypeKind::Int, 1, {"seq"}, {}});
}

bool SemanticAnalyzer::analyze(Program& program) {
    program.accept(*this);
    return !diag_.has_errors();
}

void SemanticAnalyzer::visit(LiteralExpr&) {}

void SemanticAnalyzer::visit(IdentifierExpr& node) {
    const Symbol* sym = symbols_.lookup(node.name());
    if (!sym) {
        diag_.report(DiagnosticLevel::Error, ErrorCategory::Identifier,
                     "Use of undeclared identifier '" + node.name() + "'", node.span());
    }
}

void SemanticAnalyzer::visit(UnaryExpr& node) {
    if (node.operand()) {
        node.operand()->accept(*this);
    }
}

void SemanticAnalyzer::visit(BinaryExpr& node) {
    if (node.left()) node.left()->accept(*this);
    if (node.right()) node.right()->accept(*this);
}

void SemanticAnalyzer::visit(AssignExpr& node) {
    if (node.target()) node.target()->accept(*this);
    if (node.value()) node.value()->accept(*this);
}

void SemanticAnalyzer::visit(CallExpr& node) {
    const Symbol* sym = symbols_.lookup(node.callee());
    if (!sym) {
        diag_.report(DiagnosticLevel::Error, ErrorCategory::Identifier,
                     "Call to undeclared function '" + node.callee() + "'", node.span());
    } else if (sym->kind != SymbolKind::Function) {
        diag_.report(DiagnosticLevel::Error, ErrorCategory::Type,
                     "Identifier '" + node.callee() + "' is not a function", node.span());
    } else {
        if (node.args().size() != sym->arity) {
            diag_.report(DiagnosticLevel::Error, ErrorCategory::Type,
                         "Function '" + node.callee() + "' expects " + std::to_string(sym->arity) +
                         " arguments, but " + std::to_string(node.args().size()) + " were provided",
                         node.span());
        }
    }

    for (const auto& arg : node.args()) {
        if (arg) arg->accept(*this);
    }
}

void SemanticAnalyzer::visit(IndexExpr& node) {
    if (node.sequence()) node.sequence()->accept(*this);
    if (node.index()) node.index()->accept(*this);
}

void SemanticAnalyzer::visit(SliceExpr& node) {
    if (node.sequence()) node.sequence()->accept(*this);
    if (node.start()) node.start()->accept(*this);
    if (node.end()) node.end()->accept(*this);
}

void SemanticAnalyzer::visit(MethodCallExpr& node) {
    if (node.object()) node.object()->accept(*this);
    for (const auto& arg : node.args()) {
        if (arg) arg->accept(*this);
    }
}

void SemanticAnalyzer::visit(CommaExpr& node) {
    for (const auto& expr : node.expressions()) {
        if (expr) expr->accept(*this);
    }
}

void SemanticAnalyzer::visit(ExprStmt& node) {
    if (node.expr()) node.expr()->accept(*this);
}

void SemanticAnalyzer::visit(BlockStmt& node) {
    symbols_.enter_scope();
    for (const auto& stmt : node.statements()) {
        if (stmt) stmt->accept(*this);
    }
    symbols_.exit_scope();
}

void SemanticAnalyzer::visit(VarDeclStmt& node) {
    for (const auto& var : node.variables()) {
        if (symbols_.lookup_local(var.name)) {
            diag_.report(DiagnosticLevel::Error, ErrorCategory::Identifier,
                         "Redeclaration of variable '" + var.name + "' in the same scope", var.span);
        } else {
            symbols_.define(Symbol{var.name, SymbolKind::Variable, node.type(), 0, {}, var.span});
        }

        if (var.init_value) {
            var.init_value->accept(*this);
        }
    }
}

void SemanticAnalyzer::visit(FunctionDeclStmt& node) {
    if (symbols_.lookup_local(node.name())) {
        diag_.report(DiagnosticLevel::Error, ErrorCategory::Identifier,
                     "Redeclaration of function '" + node.name() + "'", node.span());
    } else {
        symbols_.define(Symbol{node.name(), SymbolKind::Function, TypeKind::None,
                               node.params().size(), node.params(), node.span()});
    }

    ++function_depth_;
    symbols_.enter_scope(true);

    for (const auto& param : node.params()) {
        if (symbols_.lookup_local(param)) {
            diag_.report(DiagnosticLevel::Error, ErrorCategory::Identifier,
                         "Duplicate parameter '" + param + "' in function '" + node.name() + "'",
                         node.span());
        } else {
            symbols_.define(Symbol{param, SymbolKind::Variable, TypeKind::None, 0, {}, node.span()});
        }
    }

    if (node.body()) {
        for (const auto& stmt : node.body()->statements()) {
            if (stmt) stmt->accept(*this);
        }
    }

    symbols_.exit_scope();
    --function_depth_;
}

void SemanticAnalyzer::visit(IfStmt& node) {
    if (node.condition()) node.condition()->accept(*this);
    if (node.then_branch()) node.then_branch()->accept(*this);
    if (node.else_branch()) node.else_branch()->accept(*this);
}

void SemanticAnalyzer::visit(WhileStmt& node) {
    if (node.condition()) node.condition()->accept(*this);
    ++loop_depth_;
    if (node.body()) node.body()->accept(*this);
    --loop_depth_;
}

void SemanticAnalyzer::visit(DoWhileStmt& node) {
    ++loop_depth_;
    if (node.body()) node.body()->accept(*this);
    --loop_depth_;
    if (node.condition()) node.condition()->accept(*this);
}

void SemanticAnalyzer::visit(ForStmt& node) {
    if (node.sequence()) node.sequence()->accept(*this);

    symbols_.enter_scope();
    symbols_.define(Symbol{node.var_name(), SymbolKind::Variable, TypeKind::None, 0, {}, node.span()});

    ++loop_depth_;
    if (node.body()) {
        for (const auto& stmt : node.body()->statements()) {
            if (stmt) stmt->accept(*this);
        }
    }
    --loop_depth_;

    symbols_.exit_scope();
}

void SemanticAnalyzer::visit(ReturnStmt& node) {
    if (node.value()) {
        node.value()->accept(*this);
    }
}

void SemanticAnalyzer::visit(BreakStmt& node) {
    if (loop_depth_ == 0) {
        diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                     "'break' statement outside of loop", node.span());
    }
}

void SemanticAnalyzer::visit(ContinueStmt& node) {
    if (loop_depth_ == 0) {
        diag_.report(DiagnosticLevel::Error, ErrorCategory::Syntax,
                     "'continue' statement outside of loop", node.span());
    }
}

void SemanticAnalyzer::visit(PassStmt&) {}

void SemanticAnalyzer::visit(PrintStmt& node) {
    for (const auto& expr : node.expressions()) {
        if (expr) expr->accept(*this);
    }
}

void SemanticAnalyzer::visit(InputStmt& node) {
    for (const auto& item : node.items()) {
        const Symbol* sym = symbols_.lookup(item.var_name);
        if (!sym) {
            diag_.report(DiagnosticLevel::Error, ErrorCategory::Identifier,
                         "Use of undeclared variable '" + item.var_name + "' in input statement",
                         item.span);
        }
    }
}

void SemanticAnalyzer::visit(ImportStmt& node) {
    for (const auto& mod : node.modules()) {
        if (mod) mod->accept(*this);
    }
}

void SemanticAnalyzer::visit(Program& node) {
    for (const auto& stmt : node.statements()) {
        if (stmt) stmt->accept(*this);
    }
}

} // namespace execore
