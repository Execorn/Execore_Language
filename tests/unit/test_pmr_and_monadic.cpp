#include "execore/common/result.hpp"
#include "execore/common/pmr_resource.hpp"
#include "execore/frontend/token_soa.hpp"
#include "execore/runtime/environment.hpp"
#include <iostream>
#include <cassert>
#include <string>
#include <vector>

void test_monadic_result() {
    auto parse_int = [](std::string_view s) -> execore::Result<int, std::string> {
        if (s.empty()) return execore::Result<int, std::string>(std::string("empty input"));
        int val = 0;
        for (char c : s) {
            if (c < '0' || c > '9') return execore::Result<int, std::string>(std::string("not a digit"));
            val = val * 10 + (c - '0');
        }
        return execore::Result<int, std::string>(val);
    };

    auto double_val = [](int x) { return x * 2; };

    // Success path
    auto r1 = parse_int("42").transform(double_val);
    assert(r1.is_ok());
    assert(r1.value() == 84);

    // Chaining with and_then
    auto check_positive = [](int x) -> execore::Result<int, std::string> {
        if (x > 0) return execore::Result<int, std::string>(x);
        return execore::Result<int, std::string>(std::string("non-positive"));
    };

    auto r2 = parse_int("100").and_then(check_positive);
    assert(r2.is_ok());
    assert(*r2 == 100);

    // Failure path
    auto r3 = parse_int("invalid").transform(double_val);
    assert(r3.is_err());
    assert(r3.error() == "not a digit");

    // or_else fallback
    auto r4 = parse_int("").or_else([](const std::string&) -> execore::Result<int, std::string> {
        return execore::Result<int, std::string>(0);
    });
    assert(r4.is_ok());
    assert(r4.value() == 0);

    std::cout << "[PASS] test_monadic_result\n";
}

void test_pmr_allocator() {
    execore::MonotonicArenaResource arena(1024);

    std::pmr::vector<int> numbers(&arena);
    for (int i = 0; i < 100; ++i) {
        numbers.push_back(i * 10);
    }

    assert(numbers.size() == 100);
    assert(numbers[5] == 50);
    assert(arena.total_allocated() > 0);

    size_t allocated_before = arena.total_allocated();
    arena.release();
    assert(arena.total_allocated() == 0);

    std::cout << "[PASS] test_pmr_allocator (allocated and released " << allocated_before << " bytes)\n";
}

void test_token_buffer_soa() {
    execore::TokenBufferSoA buffer;
    buffer.push_back(execore::TokenKind::KwDef, "def", {});
    buffer.push_back(execore::TokenKind::Identifier, "foo", {});
    buffer.push_back(execore::TokenKind::LeftParen, "(", {});
    buffer.push_back(execore::TokenKind::RightParen, ")", {});
    buffer.push_back(execore::TokenKind::Colon, ":", {});

    assert(buffer.size() == 5);
    assert(buffer.kind(0) == execore::TokenKind::KwDef);
    assert(buffer.lexeme(1) == "foo");
    assert(buffer.count_tokens(execore::TokenKind::Colon) == 1);
    assert(buffer.find_next(execore::TokenKind::RightParen) == 3);

    // Verify span view
    auto kinds = buffer.kinds();
    assert(kinds.size() == 5);
    assert(kinds[0] == execore::TokenKind::KwDef);

    std::cout << "[PASS] test_token_buffer_soa\n";
}

void test_cacheline_alignment() {
    execore::ThreadExecutionContext ctx;
    ctx.step_count = 1000;
    ctx.call_depth = 5;
    ctx.interrupt_requested = false;

    uintptr_t addr = reinterpret_cast<uintptr_t>(&ctx);
    assert(addr % alignof(execore::ThreadExecutionContext) == 0);
    assert(alignof(execore::ThreadExecutionContext) >= 64);

    std::cout << "[PASS] test_cacheline_alignment\n";
}

int main() {
    test_monadic_result();
    test_pmr_allocator();
    test_token_buffer_soa();
    test_cacheline_alignment();
    std::cout << "All Milestone 2 Idiom & Memory tests passed successfully!\n";
    return 0;
}
