#include "execore/runtime/environment.hpp"

namespace execore {

void Environment::define(const std::string& name, Value val, TypeKind type) {
    bindings_[name] = VariableBinding{std::move(val), type};
}

bool Environment::assign(const std::string& name, Value val) {
    auto it = bindings_.find(name);
    if (it != bindings_.end()) {
        it->second.value = std::move(val);
        return true;
    }
    if (parent_) {
        return parent_->assign(name, std::move(val));
    }
    return false;
}

std::optional<Value> Environment::get(const std::string& name) const {
    auto it = bindings_.find(name);
    if (it != bindings_.end()) {
        return it->second.value;
    }
    if (parent_) {
        return parent_->get(name);
    }
    return std::nullopt;
}

bool Environment::contains(const std::string& name) const {
    if (bindings_.find(name) != bindings_.end()) return true;
    return parent_ ? parent_->contains(name) : false;
}

} // namespace execore
