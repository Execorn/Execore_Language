#pragma once

#include "execore/runtime/value.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include <optional>

#include <new>

#ifdef __cpp_lib_hardware_interference_size
using std::hardware_destructive_interference_size;
#else
constexpr size_t hardware_destructive_interference_size = 64;
#endif

namespace execore {

struct alignas(hardware_destructive_interference_size) ThreadExecutionContext {
    uint64_t step_count{0};
    uint64_t call_depth{0};
    bool interrupt_requested{false};
};

static_assert(alignof(ThreadExecutionContext) >= hardware_destructive_interference_size,
              "ThreadExecutionContext must be aligned to hardware_destructive_interference_size to prevent false sharing");

struct VariableBinding {
    Value value;
    TypeKind declared_type{TypeKind::None};
};

class Environment : public std::enable_shared_from_this<Environment> {
public:
    explicit Environment(std::shared_ptr<Environment> parent = nullptr)
        : parent_(std::move(parent)) {}

    void define(const std::string& name, Value val, TypeKind type = TypeKind::None);
    bool assign(const std::string& name, Value val);
    [[nodiscard]] std::optional<Value> get(const std::string& name) const;
    [[nodiscard]] bool contains(const std::string& name) const;

    [[nodiscard]] std::shared_ptr<Environment> parent() const noexcept { return parent_; }

    // Clears bindings and detaches parent to break mutual ownership cycles with function closures
    void clear() noexcept {
        bindings_.clear();
        parent_.reset();
    }

private:
    std::shared_ptr<Environment> parent_;
    std::unordered_map<std::string, VariableBinding> bindings_;
};

} // namespace execore
