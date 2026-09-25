#pragma once

#include "execore/ast/statements.hpp"
#include "execore/runtime/value.hpp"
#include <string>
#include <vector>
#include <memory>

namespace execore {

class Environment;

class StringObject {
public:
    explicit StringObject(std::string str) : data_(std::move(str)) {}

    [[nodiscard]] const std::string& data() const noexcept { return data_; }
    [[nodiscard]] std::string& data() noexcept { return data_; }
    [[nodiscard]] size_t length() const noexcept { return data_.size(); }

private:
    std::string data_;
};

class ListObject {
public:
    explicit ListObject(std::vector<Value> elements = {}) : elements_(std::move(elements)) {}

    [[nodiscard]] const std::vector<Value>& elements() const noexcept { return elements_; }
    [[nodiscard]] std::vector<Value>& elements() noexcept { return elements_; }
    [[nodiscard]] size_t length() const noexcept { return elements_.size(); }

    void append(Value val);
    void insert(size_t index, Value val);
    Value remove(size_t index);

private:
    std::vector<Value> elements_;
};

class FunctionObject {
public:
    FunctionObject(std::string name, std::vector<std::string> params,
                   const BlockStmt* body, std::shared_ptr<Environment> closure)
        : name_(std::move(name)), params_(std::move(params)),
          body_(body), closure_(std::move(closure)) {}

    [[nodiscard]] const std::string& name() const noexcept { return name_; }
    [[nodiscard]] const std::vector<std::string>& params() const noexcept { return params_; }
    [[nodiscard]] const BlockStmt* body() const noexcept { return body_; }
    [[nodiscard]] std::shared_ptr<Environment> closure() const noexcept { return closure_; }

private:
    std::string name_;
    std::vector<std::string> params_;
    const BlockStmt* body_;
    std::shared_ptr<Environment> closure_;
};

} // namespace execore
