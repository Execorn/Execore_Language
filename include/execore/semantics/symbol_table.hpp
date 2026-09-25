#pragma once

#include "execore/common/types.hpp"
#include "execore/common/source_location.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>

namespace execore {

enum class SymbolKind : uint8_t {
    Variable,
    Function
};

struct Symbol {
    std::string name;
    SymbolKind kind{SymbolKind::Variable};
    TypeKind type{TypeKind::None};
    size_t arity{0};
    std::vector<std::string> params;
    SourceSpan span;
};

class Scope {
public:
    explicit Scope(std::shared_ptr<Scope> parent = nullptr, bool is_function_scope = false)
        : parent_(std::move(parent)), is_function_scope_(is_function_scope) {}

    bool define(Symbol symbol);
    [[nodiscard]] const Symbol* lookup(const std::string& name) const;
    [[nodiscard]] const Symbol* lookup_local(const std::string& name) const;

    [[nodiscard]] std::shared_ptr<Scope> parent() const noexcept { return parent_; }
    [[nodiscard]] bool is_function_scope() const noexcept { return is_function_scope_; }

private:
    std::shared_ptr<Scope> parent_;
    bool is_function_scope_{false};
    std::unordered_map<std::string, Symbol> symbols_;
};

class SymbolTable {
public:
    SymbolTable();

    void enter_scope(bool is_function_scope = false);
    void exit_scope();

    bool define(Symbol symbol);
    [[nodiscard]] const Symbol* lookup(const std::string& name) const;
    [[nodiscard]] const Symbol* lookup_local(const std::string& name) const;

    bool define_builtin(Symbol symbol);

    [[nodiscard]] std::shared_ptr<Scope> current_scope() const noexcept { return current_scope_; }
    [[nodiscard]] std::shared_ptr<Scope> global_scope() const noexcept { return global_scope_; }
    [[nodiscard]] std::shared_ptr<Scope> builtin_scope() const noexcept { return builtin_scope_; }

private:
    std::shared_ptr<Scope> builtin_scope_;
    std::shared_ptr<Scope> global_scope_;
    std::shared_ptr<Scope> current_scope_;
};

} // namespace execore
