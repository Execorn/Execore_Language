#include "execore/runtime/interpreter.hpp"
#include "execore/runtime/object.hpp"
#include "execore/frontend/lexer.hpp"
#include "execore/frontend/parser.hpp"
#include <sstream>

namespace execore {

Interpreter::Interpreter(DiagnosticEngine& diag, SourceManager* sm,
                         std::istream& in, std::ostream& out)
    : diag_(diag), sm_(sm), in_(in), out_(out) {
    global_env_ = create_environment();
    current_env_ = global_env_;
}

Interpreter::~Interpreter() {
    // Function objects capture their lexical environment by shared_ptr, and environments store
    // those function objects in variable bindings. This creates cyclic reference graphs that
    // prevent refcounts from reaching zero. Breaking all bindings here reclaims all memory cleanly.
    for (auto& weak_env : all_environments_) {
        if (auto env = weak_env.lock()) {
            env->clear();
        }
    }
    all_environments_.clear();
    if (global_env_) {
        global_env_->clear();
        global_env_.reset();
    }
    current_env_.reset();
}

std::shared_ptr<Environment> Interpreter::create_environment(std::shared_ptr<Environment> parent) {
    auto env = std::make_shared<Environment>(std::move(parent));
    all_environments_.push_back(env);
    return env;
}

int Interpreter::execute(Program& program) {
    try {
        program.accept(*this);
    } catch (const ReturnSignal& sig) {
        if (sig.value.is_int()) {
            return static_cast<int>(sig.value.as_int());
        }
        return 0;
    } catch (const std::exception& e) {
        diag_.report(DiagnosticLevel::Error, ErrorCategory::Runtime, e.what());
        return 1;
    }
    return 0;
}

Value Interpreter::evaluate(Expr& expr) {
    expr.accept(*this);
    return std::move(last_evaluated_value_);
}

void Interpreter::execute_statement(Stmt& stmt) {
    stmt.accept(*this);
}

void Interpreter::execute_block(const BlockStmt& block, std::shared_ptr<Environment> env) {
    auto previous = current_env_;
    current_env_ = std::move(env);
    try {
        for (const auto& stmt : block.statements()) {
            if (stmt) execute_statement(*stmt);
        }
    } catch (...) {
        current_env_ = std::move(previous);
        throw;
    }
    current_env_ = std::move(previous);
}

void Interpreter::visit(LiteralExpr& node) {
    switch (node.literal_type()) {
        case TypeKind::Int:
            last_evaluated_value_ = Value(std::stoll(node.value()));
            break;
        case TypeKind::Float:
            last_evaluated_value_ = Value(std::stod(node.value()));
            break;
        case TypeKind::Char:
            last_evaluated_value_ = Value(node.value().empty() ? '\0' : node.value()[0]);
            break;
        case TypeKind::Str:
            last_evaluated_value_ = Value(node.value());
            break;
        default:
            last_evaluated_value_ = Value();
            break;
    }
}

void Interpreter::visit(IdentifierExpr& node) {
    auto val = current_env_->get(node.name());
    if (!val) {
        throw std::runtime_error("IdentifierError: undefined identifier '" + node.name() + "'");
    }
    last_evaluated_value_ = *val;
}

void Interpreter::visit(UnaryExpr& node) {
    Value operand = evaluate(*node.operand());
    switch (node.op()) {
        case UnaryOp::Not:
            last_evaluated_value_ = !operand;
            break;
        case UnaryOp::Minus:
            last_evaluated_value_ = -operand;
            break;
        case UnaryOp::Plus:
            last_evaluated_value_ = operand;
            break;
    }
}

void Interpreter::visit(BinaryExpr& node) {
    // Short-circuit logical operators
    if (node.op() == BinaryOp::And) {
        Value left = evaluate(*node.left());
        if (!left.is_truthy()) {
            last_evaluated_value_ = left;
            return;
        }
        last_evaluated_value_ = evaluate(*node.right());
        return;
    }
    if (node.op() == BinaryOp::Or) {
        Value left = evaluate(*node.left());
        if (left.is_truthy()) {
            last_evaluated_value_ = left;
            return;
        }
        last_evaluated_value_ = evaluate(*node.right());
        return;
    }

    Value left = evaluate(*node.left());
    Value right = evaluate(*node.right());

    switch (node.op()) {
        case BinaryOp::Add:          last_evaluated_value_ = left + right; break;
        case BinaryOp::Sub:          last_evaluated_value_ = left - right; break;
        case BinaryOp::Mul:          last_evaluated_value_ = left * right; break;
        case BinaryOp::Div:          last_evaluated_value_ = left / right; break;
        case BinaryOp::Mod:          last_evaluated_value_ = left % right; break;
        case BinaryOp::Equal:        last_evaluated_value_ = Value(static_cast<int64_t>(left == right ? 1 : 0)); break;
        case BinaryOp::NotEqual:     last_evaluated_value_ = Value(static_cast<int64_t>(left != right ? 1 : 0)); break;
        case BinaryOp::Less:         last_evaluated_value_ = Value(static_cast<int64_t>(left < right ? 1 : 0)); break;
        case BinaryOp::LessEqual:    last_evaluated_value_ = Value(static_cast<int64_t>(left <= right ? 1 : 0)); break;
        case BinaryOp::Greater:      last_evaluated_value_ = Value(static_cast<int64_t>(left > right ? 1 : 0)); break;
        case BinaryOp::GreaterEqual: last_evaluated_value_ = Value(static_cast<int64_t>(left >= right ? 1 : 0)); break;
        case BinaryOp::In:           last_evaluated_value_ = Value(static_cast<int64_t>(right.contains(left) ? 1 : 0)); break;
        default: break;
    }
}

void Interpreter::visit(AssignExpr& node) {
    Value val = evaluate(*node.value());

    if (auto* id = dynamic_cast<IdentifierExpr*>(node.target())) {
        if (node.op() == AssignOp::Assign) {
            if (!current_env_->assign(id->name(), val)) {
                current_env_->define(id->name(), val);
            }
            last_evaluated_value_ = val;
            return;
        }

        auto current_val = current_env_->get(id->name());
        if (!current_val) {
            throw std::runtime_error("IdentifierError: variable '" + id->name() + "' not initialized");
        }

        Value new_val;
        switch (node.op()) {
            case AssignOp::AddAssign:     new_val = *current_val + val; break;
            case AssignOp::SubAssign:     new_val = *current_val - val; break;
            case AssignOp::MulAssign:     new_val = *current_val * val; break;
            case AssignOp::DivAssign:     new_val = *current_val / val; break;
            case AssignOp::ModAssign:     new_val = *current_val % val; break;
            default:                      new_val = val; break;
        }

        current_env_->assign(id->name(), new_val);
        last_evaluated_value_ = new_val;
        return;
    }

    if (auto* idx_expr = dynamic_cast<IndexExpr*>(node.target())) {
        Value seq = evaluate(*idx_expr->sequence());
        Value idx = evaluate(*idx_expr->index());
        seq.set_item(idx.as_int(), val);
        last_evaluated_value_ = val;
        return;
    }

    throw std::runtime_error("SyntaxError: invalid assignment target");
}

void Interpreter::visit(CallExpr& node) {
    std::vector<Value> args;
    args.reserve(node.args().size());
    for (const auto& arg : node.args()) {
        args.push_back(evaluate(*arg));
    }

    if (builtins_.has_builtin(node.callee())) {
        last_evaluated_value_ = builtins_.call(node.callee(), args);
        return;
    }

    auto func_val = current_env_->get(node.callee());
    if (!func_val || !func_val->is_function()) {
        throw std::runtime_error("IdentifierError: '" + node.callee() + "' is not a callable function");
    }

    auto func_obj = func_val->as_function_object();
    if (args.size() != func_obj->params().size()) {
        throw std::runtime_error("TypeError: function '" + node.callee() + "' takes " +
                                 std::to_string(func_obj->params().size()) + " arguments, but " +
                                 std::to_string(args.size()) + " were given");
    }

    // Bind activation record to the captured closure environment rather than the call-site
    // environment so that lexical scoping is preserved.
    auto func_env = create_environment(func_obj->closure());
    for (size_t i = 0; i < args.size(); ++i) {
        func_env->define(func_obj->params()[i], std::move(args[i]));
    }

    try {
        if (func_obj->body()) {
            execute_block(*func_obj->body(), std::move(func_env));
        }
        last_evaluated_value_ = Value();
    } catch (const ReturnSignal& sig) {
        last_evaluated_value_ = sig.value;
    }
}

void Interpreter::visit(IndexExpr& node) {
    Value seq = evaluate(*node.sequence());
    Value idx = evaluate(*node.index());
    last_evaluated_value_ = seq.get_item(idx.as_int());
}

void Interpreter::visit(SliceExpr& node) {
    Value seq = evaluate(*node.sequence());
    std::optional<int64_t> start;
    std::optional<int64_t> end;

    if (node.start()) {
        start = evaluate(*node.start()).as_int();
    }
    if (node.end()) {
        end = evaluate(*node.end()).as_int();
    }

    last_evaluated_value_ = seq.slice(start, end);
}

void Interpreter::visit(MethodCallExpr& node) {
    Value obj = evaluate(*node.object());
    std::vector<Value> args;
    args.reserve(node.args().size());
    for (const auto& arg : node.args()) {
        args.push_back(evaluate(*arg));
    }
    last_evaluated_value_ = obj.call_method(node.method_name(), args);
}

void Interpreter::visit(CommaExpr& node) {
    Value res;
    for (const auto& expr : node.expressions()) {
        res = evaluate(*expr);
    }
    last_evaluated_value_ = std::move(res);
}

void Interpreter::visit(ExprStmt& node) {
    if (node.expr()) {
        evaluate(*node.expr());
    }
}

void Interpreter::visit(BlockStmt& node) {
    execute_block(node, create_environment(current_env_));
}

void Interpreter::visit(VarDeclStmt& node) {
    for (const auto& var : node.variables()) {
        Value init_val;
        if (var.init_value) {
            init_val = evaluate(*var.init_value);
        } else {
            switch (node.type()) {
                case TypeKind::Int:   init_val = Value(int64_t{0}); break;
                case TypeKind::Float: init_val = Value(double{0.0}); break;
                case TypeKind::Char:  init_val = Value('\0'); break;
                case TypeKind::Str:   init_val = Value(std::string{}); break;
                case TypeKind::List:  init_val = Value::make_list(); break;
                default:              init_val = Value(); break;
            }
        }
        current_env_->define(var.name, std::move(init_val), node.type());
    }
}

void Interpreter::visit(FunctionDeclStmt& node) {
    auto func_obj = std::make_shared<FunctionObject>(
        node.name(), node.params(), node.body(), current_env_
    );
    current_env_->define(node.name(), Value(func_obj), TypeKind::Function);
}

void Interpreter::visit(IfStmt& node) {
    Value cond = evaluate(*node.condition());
    if (cond.is_truthy()) {
        if (node.then_branch()) {
            node.then_branch()->accept(*this);
        }
    } else if (node.else_branch()) {
        node.else_branch()->accept(*this);
    }
}

void Interpreter::visit(WhileStmt& node) {
    while (evaluate(*node.condition()).is_truthy()) {
        try {
            if (node.body()) {
                node.body()->accept(*this);
            }
        } catch (const BreakSignal&) {
            break;
        } catch (const ContinueSignal&) {
            continue;
        }
    }
}

void Interpreter::visit(DoWhileStmt& node) {
    do {
        try {
            if (node.body()) {
                node.body()->accept(*this);
            }
        } catch (const BreakSignal&) {
            break;
        } catch (const ContinueSignal&) {
            continue;
        }
    } while (evaluate(*node.condition()).is_truthy());
}

void Interpreter::visit(ForStmt& node) {
    Value seq = evaluate(*node.sequence());
    auto loop_env = create_environment(current_env_);

    if (seq.is_string()) {
        for (char c : seq.as_string()) {
            loop_env->define(node.var_name(), Value(c));
            auto prev = current_env_;
            current_env_ = loop_env;
            try {
                if (node.body()) {
                    for (const auto& stmt : node.body()->statements()) {
                        if (stmt) execute_statement(*stmt);
                    }
                }
            } catch (const BreakSignal&) {
                current_env_ = prev;
                break;
            } catch (const ContinueSignal&) {
                current_env_ = prev;
                continue;
            }
            current_env_ = prev;
        }
    } else if (seq.is_list()) {
        auto list_obj = seq.as_list_object();
        for (size_t i = 0; i < list_obj->length(); ++i) {
            loop_env->define(node.var_name(), list_obj->elements()[i]);
            auto prev = current_env_;
            current_env_ = loop_env;
            try {
                if (node.body()) {
                    for (const auto& stmt : node.body()->statements()) {
                        if (stmt) execute_statement(*stmt);
                    }
                }
            } catch (const BreakSignal&) {
                current_env_ = prev;
                break;
            } catch (const ContinueSignal&) {
                current_env_ = prev;
                continue;
            }
            current_env_ = prev;
        }
    } else {
        throw std::runtime_error("TypeError: '" + seq.type_name() + "' object is not iterable");
    }
}

void Interpreter::visit(ReturnStmt& node) {
    Value ret_val;
    if (node.value()) {
        ret_val = evaluate(*node.value());
    }
    throw ReturnSignal{std::move(ret_val)};
}

void Interpreter::visit(BreakStmt&) {
    throw BreakSignal{};
}

void Interpreter::visit(ContinueStmt&) {
    throw ContinueSignal{};
}

void Interpreter::visit(PassStmt&) {}

void Interpreter::visit(PrintStmt& node) {
    for (size_t i = 0; i < node.expressions().size(); ++i) {
        if (!node.is_raw() && i > 0) {
            out_ << " ";
        }
        Value val = evaluate(*node.expressions()[i]);
        out_ << val.to_string();
    }
    if (!node.is_raw()) {
        out_ << "\n";
    }
    out_.flush();
}

void Interpreter::visit(InputStmt& node) {
    for (const auto& item : node.items()) {
        if (!item.prompt.empty()) {
            out_ << item.prompt;
            out_.flush();
        }

        std::string line;
        if (!std::getline(in_, line)) {
            line = "";
        }

        Value val;
        auto var_opt = current_env_->get(item.var_name);
        TypeKind var_type = var_opt ? var_opt->type() : TypeKind::Str;

        switch (var_type) {
            case TypeKind::Int:
                val = Value(std::stoll(line.empty() ? "0" : line));
                break;
            case TypeKind::Float:
                val = Value(std::stod(line.empty() ? "0.0" : line));
                break;
            case TypeKind::Char:
                val = Value(line.empty() ? '\0' : line[0]);
                break;
            default:
                val = Value(line);
                break;
        }

        current_env_->assign(item.var_name, std::move(val));
    }
}

void Interpreter::visit(ImportStmt& node) {
    if (!sm_) {
        throw std::runtime_error("SystemError: cannot import module, SourceManager not configured");
    }

    for (const auto& mod_expr : node.modules()) {
        Value mod_val = evaluate(*mod_expr);
        std::string mod_name = mod_val.to_string();

        if (loaded_modules_.find(mod_name) != loaded_modules_.end()) {
            continue; // Already imported
        }
        loaded_modules_.insert(mod_name);

        auto src_opt = sm_->load_file(mod_name);
        if (!src_opt) {
            throw std::runtime_error("SystemError: failed to load module '" + mod_name + "'");
        }

        Lexer lexer(*src_opt, mod_name, diag_);
        Parser parser(lexer, diag_);
        auto program = parser.parse_program();

        if (diag_.has_errors()) {
            throw std::runtime_error("SyntaxError in imported module '" + mod_name + "'");
        }

        program->accept(*this);
    }
}

void Interpreter::visit(Program& node) {
    for (const auto& stmt : node.statements()) {
        if (stmt) execute_statement(*stmt);
    }
}

} // namespace execore
