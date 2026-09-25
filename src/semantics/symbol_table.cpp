#include "execore/semantics/symbol_table.hpp"

namespace execore {

bool Scope::define(Symbol symbol) {
    auto it = symbols_.find(symbol.name);
    if (it != symbols_.end()) {
        return false;
    }
    symbols_[symbol.name] = std::move(symbol);
    return true;
}

const Symbol* Scope::lookup(const std::string& name) const {
    const Scope* curr = this;
    while (curr) {
        auto it = curr->symbols_.find(name);
        if (it != curr->symbols_.end()) {
            return &it->second;
        }
        curr = curr->parent_.get();
    }
    return nullptr;
}

const Symbol* Scope::lookup_local(const std::string& name) const {
    auto it = symbols_.find(name);
    return (it != symbols_.end()) ? &it->second : nullptr;
}

SymbolTable::SymbolTable() {
    // Builtins live in an enclosing root scope above global_scope to let global variable
    // declarations shadow builtin function names without triggering redeclaration errors.
    builtin_scope_ = std::make_shared<Scope>(nullptr, false);
    global_scope_ = std::make_shared<Scope>(builtin_scope_, false);
    current_scope_ = global_scope_;
}

void SymbolTable::enter_scope(bool is_function_scope) {
    current_scope_ = std::make_shared<Scope>(current_scope_, is_function_scope);
}

void SymbolTable::exit_scope() {
    if (current_scope_->parent() && current_scope_ != global_scope_) {
        current_scope_ = current_scope_->parent();
    }
}

bool SymbolTable::define_builtin(Symbol symbol) {
    return builtin_scope_->define(std::move(symbol));
}

bool SymbolTable::define(Symbol symbol) {
    return current_scope_->define(std::move(symbol));
}

const Symbol* SymbolTable::lookup(const std::string& name) const {
    return current_scope_->lookup(name);
}

const Symbol* SymbolTable::lookup_local(const std::string& name) const {
    return current_scope_->lookup_local(name);
}

} // namespace execore
