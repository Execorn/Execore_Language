#include "execore/runtime/value.hpp"
#include "execore/runtime/object.hpp"
#include <sstream>
#include <cmath>
#include <stdexcept>
#include <algorithm>

namespace execore {

Value::Value(std::string str) : data_(std::make_shared<StringObject>(std::move(str))) {}
Value::Value(const char* str) : data_(std::make_shared<StringObject>(std::string(str))) {}
Value::Value(std::shared_ptr<StringObject> str_obj) : data_(std::move(str_obj)) {}
Value::Value(std::shared_ptr<ListObject> list_obj) : data_(std::move(list_obj)) {}
Value::Value(std::shared_ptr<FunctionObject> func_obj) : data_(std::move(func_obj)) {}

Value Value::make_list(std::vector<Value> elements) {
    return Value(std::make_shared<ListObject>(std::move(elements)));
}

TypeKind Value::type() const noexcept {
    if (is_none()) return TypeKind::None;
    if (is_int()) return TypeKind::Int;
    if (is_float()) return TypeKind::Float;
    if (is_char()) return TypeKind::Char;
    if (is_string()) return TypeKind::Str;
    if (is_list()) return TypeKind::List;
    if (is_function()) return TypeKind::Function;
    return TypeKind::None;
}

std::string Value::type_name() const noexcept {
    return std::string(execore::to_string(type()));
}

int64_t Value::as_int() const {
    if (is_int()) return std::get<int64_t>(data_);
    if (is_float()) return static_cast<int64_t>(std::get<double>(data_));
    if (is_char()) return static_cast<int64_t>(std::get<char>(data_));
    throw std::runtime_error("TypeError: cannot convert " + type_name() + " to int");
}

double Value::as_float() const {
    if (is_float()) return std::get<double>(data_);
    if (is_int()) return static_cast<double>(std::get<int64_t>(data_));
    if (is_char()) return static_cast<double>(std::get<char>(data_));
    throw std::runtime_error("TypeError: cannot convert " + type_name() + " to float");
}

char Value::as_char() const {
    if (is_char()) return std::get<char>(data_);
    if (is_int()) return static_cast<char>(std::get<int64_t>(data_));
    if (is_string()) {
        const auto& s = as_string();
        if (!s.empty()) return s[0];
    }
    throw std::runtime_error("TypeError: cannot convert " + type_name() + " to char");
}

const std::string& Value::as_string() const {
    if (is_string()) return std::get<std::shared_ptr<StringObject>>(data_)->data();
    throw std::runtime_error("TypeError: expected string, got " + type_name());
}

std::shared_ptr<StringObject> Value::as_string_object() const {
    if (is_string()) return std::get<std::shared_ptr<StringObject>>(data_);
    throw std::runtime_error("TypeError: expected string, got " + type_name());
}

std::shared_ptr<ListObject> Value::as_list_object() const {
    if (is_list()) return std::get<std::shared_ptr<ListObject>>(data_);
    throw std::runtime_error("TypeError: expected list, got " + type_name());
}

std::shared_ptr<FunctionObject> Value::as_function_object() const {
    if (is_function()) return std::get<std::shared_ptr<FunctionObject>>(data_);
    throw std::runtime_error("TypeError: expected function, got " + type_name());
}

bool Value::is_truthy() const noexcept {
    if (is_none()) return false;
    if (is_int()) return std::get<int64_t>(data_) != 0;
    if (is_float()) return std::get<double>(data_) != 0.0;
    if (is_char()) return std::get<char>(data_) != '\0';
    if (is_string()) return !std::get<std::shared_ptr<StringObject>>(data_)->data().empty();
    if (is_list()) return !std::get<std::shared_ptr<ListObject>>(data_)->elements().empty();
    if (is_function()) return true;
    return false;
}

std::string Value::to_string() const {
    if (is_none()) return "None";
    if (is_int()) return std::to_string(std::get<int64_t>(data_));
    if (is_float()) {
        std::ostringstream oss;
        oss << std::get<double>(data_);
        return oss.str();
    }
    if (is_char()) return std::string(1, std::get<char>(data_));
    if (is_string()) return std::get<std::shared_ptr<StringObject>>(data_)->data();
    if (is_list()) {
        std::string res = "[";
        const auto& elems = std::get<std::shared_ptr<ListObject>>(data_)->elements();
        for (size_t i = 0; i < elems.size(); ++i) {
            if (i > 0) res += ", ";
            if (elems[i].is_string()) {
                res += "\"" + elems[i].to_string() + "\"";
            } else if (elems[i].is_char()) {
                res += "'" + elems[i].to_string() + "'";
            } else {
                res += elems[i].to_string();
            }
        }
        res += "]";
        return res;
    }
    if (is_function()) {
        return "<function " + std::get<std::shared_ptr<FunctionObject>>(data_)->name() + ">";
    }
    return "unknown";
}

Value Value::operator+(const Value& other) const {
    if (is_int() && other.is_int()) {
        return Value(as_int() + other.as_int());
    }
    if (is_number() && other.is_number()) {
        return Value(as_float() + other.as_float());
    }
    if (is_string() || other.is_string()) {
        return Value(to_string() + other.to_string());
    }
    if (is_list() && other.is_list()) {
        std::vector<Value> merged = as_list_object()->elements();
        const auto& other_elems = other.as_list_object()->elements();
        merged.insert(merged.end(), other_elems.begin(), other_elems.end());
        return Value::make_list(std::move(merged));
    }
    throw std::runtime_error("TypeError: unsupported operand types for +: '" + type_name() + "' and '" + other.type_name() + "'");
}

Value Value::operator-(const Value& other) const {
    if (is_int() && other.is_int()) {
        return Value(as_int() - other.as_int());
    }
    if (is_number() && other.is_number()) {
        return Value(as_float() - other.as_float());
    }
    throw std::runtime_error("TypeError: unsupported operand types for -: '" + type_name() + "' and '" + other.type_name() + "'");
}

Value Value::operator*(const Value& other) const {
    if (is_int() && other.is_int()) {
        return Value(as_int() * other.as_int());
    }
    if (is_number() && other.is_number()) {
        return Value(as_float() * other.as_float());
    }
    // String repetition
    if (is_string() && other.is_int()) {
        int64_t count = other.as_int();
        if (count <= 0) return Value(std::string{});
        std::string base = as_string();
        std::string res;
        res.reserve(base.size() * static_cast<size_t>(count));
        for (int64_t i = 0; i < count; ++i) res += base;
        return Value(res);
    }
    if (is_int() && other.is_string()) {
        return other * (*this);
    }
    // List repetition
    if (is_list() && other.is_int()) {
        int64_t count = other.as_int();
        if (count <= 0) return Value::make_list();
        const auto& base = as_list_object()->elements();
        std::vector<Value> res;
        res.reserve(base.size() * static_cast<size_t>(count));
        for (int64_t i = 0; i < count; ++i) {
            res.insert(res.end(), base.begin(), base.end());
        }
        return Value::make_list(std::move(res));
    }
    if (is_int() && other.is_list()) {
        return other * (*this);
    }
    throw std::runtime_error("TypeError: unsupported operand types for *: '" + type_name() + "' and '" + other.type_name() + "'");
}

Value Value::operator/(const Value& other) const {
    if (!is_number() || !other.is_number()) {
        throw std::runtime_error("TypeError: unsupported operand types for /: '" + type_name() + "' and '" + other.type_name() + "'");
    }
    double denom = other.as_float();
    if (denom == 0.0) {
        throw std::runtime_error("ZeroDivisionError: division by zero");
    }
    if (is_int() && other.is_int()) {
        return Value(as_int() / other.as_int());
    }
    return Value(as_float() / denom);
}

Value Value::operator%(const Value& other) const {
    if (!is_int() || !other.is_int()) {
        throw std::runtime_error("TypeError: unsupported operand types for %: '" + type_name() + "' and '" + other.type_name() + "'");
    }
    int64_t denom = other.as_int();
    if (denom == 0) {
        throw std::runtime_error("ZeroDivisionError: modulo by zero");
    }
    return Value(as_int() % denom);
}

Value Value::operator-() const {
    if (is_int()) return Value(-as_int());
    if (is_float()) return Value(-as_float());
    if (is_char()) return Value(-static_cast<int64_t>(as_char()));
    throw std::runtime_error("TypeError: unsupported operand type for unary -: '" + type_name() + "'");
}

Value Value::operator!() const {
    return Value(static_cast<int64_t>(is_truthy() ? 0 : 1));
}

bool Value::operator==(const Value& other) const {
    if (type() != other.type()) {
        if (is_number() && other.is_number()) {
            return as_float() == other.as_float();
        }
        return false;
    }
    if (is_none()) return true;
    if (is_int()) return as_int() == other.as_int();
    if (is_float()) return as_float() == other.as_float();
    if (is_char()) return as_char() == other.as_char();
    if (is_string()) return as_string() == other.as_string();
    if (is_list()) {
        const auto& l1 = as_list_object()->elements();
        const auto& l2 = other.as_list_object()->elements();
        if (l1.size() != l2.size()) return false;
        for (size_t i = 0; i < l1.size(); ++i) {
            if (l1[i] != l2[i]) return false;
        }
        return true;
    }
    if (is_function()) return as_function_object() == other.as_function_object();
    return false;
}

bool Value::operator<(const Value& other) const {
    if (is_number() && other.is_number()) {
        return as_float() < other.as_float();
    }
    if (is_string() && other.is_string()) {
        return as_string() < other.as_string();
    }
    throw std::runtime_error("TypeError: '<' not supported between instances of '" + type_name() + "' and '" + other.type_name() + "'");
}

bool Value::operator<=(const Value& other) const {
    return (*this < other) || (*this == other);
}

bool Value::operator>(const Value& other) const {
    return !(*this <= other);
}

bool Value::operator>=(const Value& other) const {
    return !(*this < other);
}

bool Value::contains(const Value& item) const {
    if (is_string()) {
        std::string needle = item.to_string();
        return as_string().find(needle) != std::string::npos;
    }
    if (is_list()) {
        const auto& elems = as_list_object()->elements();
        return std::any_of(elems.begin(), elems.end(), [&](const Value& elem) {
            return elem == item;
        });
    }
    throw std::runtime_error("TypeError: argument of type '" + type_name() + "' is not iterable");
}

Value Value::get_item(int64_t index) const {
    if (is_string()) {
        const auto& str = as_string();
        if (index < 0) index += static_cast<int64_t>(str.size());
        if (index < 0 || static_cast<size_t>(index) >= str.size()) {
            throw std::runtime_error("IndexError: string index out of range");
        }
        return Value(str[static_cast<size_t>(index)]);
    }
    if (is_list()) {
        const auto& elems = as_list_object()->elements();
        if (index < 0) index += static_cast<int64_t>(elems.size());
        if (index < 0 || static_cast<size_t>(index) >= elems.size()) {
            throw std::runtime_error("IndexError: list index out of range");
        }
        return elems[static_cast<size_t>(index)];
    }
    throw std::runtime_error("TypeError: '" + type_name() + "' object is not subscriptable");
}

void Value::set_item(int64_t index, Value val) {
    if (is_list()) {
        auto& elems = as_list_object()->elements();
        if (index < 0) index += static_cast<int64_t>(elems.size());
        if (index < 0 || static_cast<size_t>(index) >= elems.size()) {
            throw std::runtime_error("IndexError: list assignment index out of range");
        }
        elems[static_cast<size_t>(index)] = std::move(val);
        return;
    }
    throw std::runtime_error("TypeError: '" + type_name() + "' object does not support item assignment");
}

Value Value::slice(std::optional<int64_t> start_opt, std::optional<int64_t> end_opt) const {
    if (is_string()) {
        const auto& str = as_string();
        int64_t len = static_cast<int64_t>(str.size());
        int64_t start = start_opt.value_or(0);
        int64_t end = end_opt.value_or(len);

        if (start < 0) start += len;
        if (end < 0) end += len;
        start = std::clamp(start, int64_t{0}, len);
        end = std::clamp(end, int64_t{0}, len);

        if (start >= end) return Value(std::string{});
        return Value(str.substr(static_cast<size_t>(start), static_cast<size_t>(end - start)));
    }
    if (is_list()) {
        const auto& elems = as_list_object()->elements();
        int64_t len = static_cast<int64_t>(elems.size());
        int64_t start = start_opt.value_or(0);
        int64_t end = end_opt.value_or(len);

        if (start < 0) start += len;
        if (end < 0) end += len;
        start = std::clamp(start, int64_t{0}, len);
        end = std::clamp(end, int64_t{0}, len);

        if (start >= end) return Value::make_list();
        std::vector<Value> sliced(elems.begin() + start, elems.begin() + end);
        return Value::make_list(std::move(sliced));
    }
    throw std::runtime_error("TypeError: cannot slice non-sequence type '" + type_name() + "'");
}

Value Value::call_method(const std::string& method_name, const std::vector<Value>& args) {
    if (is_string()) {
        if (method_name == "len") {
            if (!args.empty()) {
                throw std::runtime_error("TypeError: len() takes no arguments (" + std::to_string(args.size()) + " given)");
            }
            return Value(static_cast<int64_t>(as_string().size()));
        }
        throw std::runtime_error("AttributeError: 'str' object has no attribute '" + method_name + "'");
    }
    if (is_list()) {
        auto list_obj = as_list_object();
        if (method_name == "len") {
            if (!args.empty()) {
                throw std::runtime_error("TypeError: len() takes no arguments (" + std::to_string(args.size()) + " given)");
            }
            return Value(static_cast<int64_t>(list_obj->length()));
        }
        if (method_name == "append") {
            if (args.size() != 1) {
                throw std::runtime_error("TypeError: append() takes exactly one argument (" + std::to_string(args.size()) + " given)");
            }
            list_obj->append(args[0]);
            return Value();
        }
        if (method_name == "insert") {
            if (args.size() != 2) {
                throw std::runtime_error("TypeError: insert() takes exactly 2 arguments (" + std::to_string(args.size()) + " given)");
            }
            int64_t idx = args[0].as_int();
            if (idx < 0) idx = 0;
            list_obj->insert(static_cast<size_t>(idx), args[1]);
            return Value();
        }
        if (method_name == "remove") {
            if (args.size() != 1) {
                throw std::runtime_error("TypeError: remove() takes exactly one argument (" + std::to_string(args.size()) + " given)");
            }
            int64_t idx = args[0].as_int();
            if (idx < 0) {
                throw std::runtime_error("IndexError: remove index out of range");
            }
            return list_obj->remove(static_cast<size_t>(idx));
        }
        throw std::runtime_error("AttributeError: 'list' object has no attribute '" + method_name + "'");
    }
    throw std::runtime_error("AttributeError: '" + type_name() + "' object has no method '" + method_name + "'");
}

std::ostream& operator<<(std::ostream& os, const Value& val) {
    os << val.to_string();
    return os;
}

} // namespace execore
