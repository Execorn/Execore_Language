#pragma once

#include "execore/runtime/value.hpp"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace execore {

using BuiltinFn = std::function<Value(const std::vector<Value>&)>;

class BuiltinRegistry {
public:
    BuiltinRegistry();

    void register_builtin(const std::string& name, size_t arity, BuiltinFn fn);
    [[nodiscard]] bool has_builtin(const std::string& name) const noexcept;
    [[nodiscard]] Value call(const std::string& name, const std::vector<Value>& args) const;

private:
    struct Entry {
        size_t arity;
        BuiltinFn fn;
    };
    std::unordered_map<std::string, Entry> builtins_;
};

} // namespace execore
