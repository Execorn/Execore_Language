#include "execore/runtime/object.hpp"
#include "execore/runtime/value.hpp"
#include "execore/common/error_code.hpp"
#include <stdexcept>

namespace execore {

void ListObject::append(Value val) {
    elements_.push_back(std::move(val));
}

void ListObject::insert(size_t index, Value val) {
    if (index > elements_.size()) {
        index = elements_.size();
    }
    elements_.insert(elements_.begin() + static_cast<std::ptrdiff_t>(index), std::move(val));
}

Value ListObject::remove(size_t index) {
    if (index >= elements_.size()) {
        throw std::runtime_error("IndexError: list remove index out of range");
    }
    Value val = std::move(elements_[index]);
    elements_.erase(elements_.begin() + static_cast<std::ptrdiff_t>(index));
    return val;
}

} // namespace execore
