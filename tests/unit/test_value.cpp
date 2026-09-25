#include "execore/runtime/value.hpp"
#include <iostream>
#include <cassert>

void test_value_arithmetic() {
    execore::Value a(int64_t{10});
    execore::Value b(int64_t{20});

    assert((a + b).as_int() == 30);
    assert((b - a).as_int() == 10);
    assert((a * b).as_int() == 200);
    assert((b / a).as_int() == 2);
    assert((b % a).as_int() == 0);

    execore::Value f(2.5);
    assert((a + f).as_float() == 12.5);

    std::cout << "[PASS] test_value_arithmetic\n";
}

void test_value_strings_and_lists() {
    execore::Value s1("hello");
    execore::Value s2(" world");
    assert((s1 + s2).to_string() == "hello world");
    assert((s1 * execore::Value(3)).to_string() == "hellohellohello");

    auto list = execore::Value::make_list({execore::Value(1), execore::Value(2)});
    (void)list.call_method("append", {execore::Value(3)});
    assert(list.call_method("len", {}).as_int() == 3);

    auto item = list.get_item(1);
    assert(item.as_int() == 2);

    auto sliced = list.slice(1, 3);
    assert(sliced.call_method("len", {}).as_int() == 2);

    std::cout << "[PASS] test_value_strings_and_lists\n";
}

int main() {
    test_value_arithmetic();
    test_value_strings_and_lists();
    std::cout << "All Value tests passed successfully!\n";
    return 0;
}
