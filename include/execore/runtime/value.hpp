#pragma once

#include "execore/common/types.hpp"
#include <variant>
#include <optional>
#include <concepts>
#include <string>
#include <vector>
#include <memory>
#include <iosfwd>

namespace execore {

class StringObject;
class ListObject;
class FunctionObject;

class Value {
public:
    using VariantType = std::variant<
        std::monostate,                         // None
        int64_t,                                // Int
        double,                                 // Float
        char,                                   // Char
        std::shared_ptr<StringObject>,          // Str
        std::shared_ptr<ListObject>,            // List
        std::shared_ptr<FunctionObject>         // Function
    >;

    Value() : data_(std::monostate{}) {}
    Value(std::monostate) : data_(std::monostate{}) {}
    template <typename T>
    requires (std::integral<T> && !std::same_as<T, bool> && !std::same_as<T, char>)
    Value(T val) : data_(static_cast<int64_t>(val)) {}

    template <typename T>
    requires (std::floating_point<T>)
    Value(T val) : data_(static_cast<double>(val)) {}
    Value(char val) : data_(val) {}
    Value(std::string str);
    Value(const char* str);
    Value(std::shared_ptr<StringObject> str_obj);
    Value(std::shared_ptr<ListObject> list_obj);
    Value(std::shared_ptr<FunctionObject> func_obj);

    static Value make_list(std::vector<Value> elements = {});

    [[nodiscard]] TypeKind type() const noexcept;
    [[nodiscard]] std::string type_name() const noexcept;

    [[nodiscard]] bool is_none() const noexcept { return std::holds_alternative<std::monostate>(data_); }
    [[nodiscard]] bool is_int() const noexcept { return std::holds_alternative<int64_t>(data_); }
    [[nodiscard]] bool is_float() const noexcept { return std::holds_alternative<double>(data_); }
    [[nodiscard]] bool is_char() const noexcept { return std::holds_alternative<char>(data_); }
    [[nodiscard]] bool is_number() const noexcept { return is_int() || is_float() || is_char(); }
    [[nodiscard]] bool is_string() const noexcept { return std::holds_alternative<std::shared_ptr<StringObject>>(data_); }
    [[nodiscard]] bool is_list() const noexcept { return std::holds_alternative<std::shared_ptr<ListObject>>(data_); }
    [[nodiscard]] bool is_function() const noexcept { return std::holds_alternative<std::shared_ptr<FunctionObject>>(data_); }

    [[nodiscard]] int64_t as_int() const;
    [[nodiscard]] double as_float() const;
    [[nodiscard]] char as_char() const;
    [[nodiscard]] const std::string& as_string() const;
    [[nodiscard]] std::shared_ptr<StringObject> as_string_object() const;
    [[nodiscard]] std::shared_ptr<ListObject> as_list_object() const;
    [[nodiscard]] std::shared_ptr<FunctionObject> as_function_object() const;

    [[nodiscard]] bool is_truthy() const noexcept;
    [[nodiscard]] std::string to_string() const;

    // Operations
    [[nodiscard]] Value operator+(const Value& other) const;
    [[nodiscard]] Value operator-(const Value& other) const;
    [[nodiscard]] Value operator*(const Value& other) const;
    [[nodiscard]] Value operator/(const Value& other) const;
    [[nodiscard]] Value operator%(const Value& other) const;

    [[nodiscard]] Value operator-() const;
    [[nodiscard]] Value operator!() const;

    [[nodiscard]] bool operator==(const Value& other) const;
    [[nodiscard]] bool operator!=(const Value& other) const { return !(*this == other); }
    [[nodiscard]] bool operator<(const Value& other) const;
    [[nodiscard]] bool operator<=(const Value& other) const;
    [[nodiscard]] bool operator>(const Value& other) const;
    [[nodiscard]] bool operator>=(const Value& other) const;

    [[nodiscard]] bool contains(const Value& item) const;

    // Index & Slicing
    [[nodiscard]] Value get_item(int64_t index) const;
    void set_item(int64_t index, Value val);
    [[nodiscard]] Value slice(std::optional<int64_t> start, std::optional<int64_t> end) const;

    // Method dispatch
    [[nodiscard]] Value call_method(const std::string& method_name, const std::vector<Value>& args);

private:
    VariantType data_;
};

std::ostream& operator<<(std::ostream& os, const Value& val);

} // namespace execore
