#include "execore/runtime/builtins.hpp"
#include "execore/runtime/object.hpp"
#include <stdexcept>

namespace execore {

BuiltinRegistry::BuiltinRegistry() {
    // chr(int) -> str
    register_builtin("chr", 1, [](const std::vector<Value>& args) -> Value {
        int64_t code = args[0].as_int();
        return Value(std::string(1, static_cast<char>(code)));
    });

    // ord(str or char) -> int
    register_builtin("ord", 1, [](const std::vector<Value>& args) -> Value {
        char c = args[0].as_char();
        return Value(static_cast<int64_t>(static_cast<unsigned char>(c)));
    });

    // type(val) -> str
    register_builtin("type", 1, [](const std::vector<Value>& args) -> Value {
        return Value(args[0].type_name());
    });

    // len(seq) -> int
    register_builtin("len", 1, [](const std::vector<Value>& args) -> Value {
        if (args[0].is_string()) {
            return Value(static_cast<int64_t>(args[0].as_string().size()));
        }
        if (args[0].is_list()) {
            return Value(static_cast<int64_t>(args[0].as_list_object()->length()));
        }
        throw std::runtime_error("TypeError: len() takes a sequence (str or list)");
    });
}

void BuiltinRegistry::register_builtin(const std::string& name, size_t arity, BuiltinFn fn) {
    builtins_[name] = Entry{arity, std::move(fn)};
}

bool BuiltinRegistry::has_builtin(const std::string& name) const noexcept {
    return builtins_.find(name) != builtins_.end();
}

Value BuiltinRegistry::call(const std::string& name, const std::vector<Value>& args) const {
    auto it = builtins_.find(name);
    if (it == builtins_.end()) {
        throw std::runtime_error("IdentifierError: builtin '" + name + "' not found");
    }
    if (args.size() != it->second.arity) {
        throw std::runtime_error("TypeError: builtin '" + name + "' expected " +
                                 std::to_string(it->second.arity) + " arguments, got " +
                                 std::to_string(args.size()));
    }
    return it->second.fn(args);
}

} // namespace execore
