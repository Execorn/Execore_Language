#include "execore/frontend/lexer.hpp"
#include "execore/runtime/value.hpp"
#include "execore/diagnostics/diagnostic_engine.hpp"
#include <iostream>
#include <sstream>
#include <cassert>
#include <string>
#include <random>
#include <vector>

void test_property_arithmetic_invariants() {
    std::mt19937_64 rng(1337);
    std::uniform_int_distribution<int64_t> dist(-1000000, 1000000);

    for (int iter = 0; iter < 1000; ++iter) {
        int64_t a_raw = dist(rng);
        int64_t b_raw = dist(rng);
        int64_t c_raw = dist(rng);

        execore::Value a(a_raw);
        execore::Value b(b_raw);
        execore::Value c(c_raw);

        // Commutativity: a + b == b + a
        assert((a + b) == (b + a));
        // Commutativity: a * b == b * a
        assert((a * b) == (b * a));
        // Associativity: (a + b) + c == a + (b + c)
        assert(((a + b) + c) == (a + (b + c)));
        // Identity: a + 0 == a
        assert((a + execore::Value(0)) == a);
        // Multiplication by 1: a * 1 == a
        assert((a * execore::Value(1)) == a);
    }
    std::cout << "[PASS] Property 1: 1,000 trials of arithmetic commutativity, associativity, and identity verified.\n";
}

void test_property_string_slicing_roundtrip() {
    std::mt19937 rng(42);
    const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::uniform_int_distribution<size_t> char_dist(0, chars.size() - 1);
    std::uniform_int_distribution<size_t> len_dist(5, 50);

    for (int iter = 0; iter < 500; ++iter) {
        size_t len = len_dist(rng);
        std::string s;
        s.reserve(len);
        for (size_t k = 0; k < len; ++k) {
            s.push_back(chars[char_dist(rng)]);
        }

        std::uniform_int_distribution<int64_t> idx_dist(0, static_cast<int64_t>(len));
        int64_t i = idx_dist(rng);
        int64_t j = idx_dist(rng);
        if (i > j) std::swap(i, j);

        execore::Value val_str(s);
        auto left = val_str.slice(0, i);
        auto mid = val_str.slice(i, j);
        auto right = val_str.slice(j, static_cast<int64_t>(len));

        execore::Value reconstructed = left + mid + right;
        assert(reconstructed.to_string() == s);
    }
    std::cout << "[PASS] Property 2: 500 trials of random string slicing decomposition and reconstruction verified.\n";
}

void test_property_token_roundtrip() {
    std::mt19937_64 rng(999);
    std::uniform_int_distribution<int64_t> dist(0, 1000000000LL);

    for (int iter = 0; iter < 500; ++iter) {
        int64_t n = dist(rng);
        std::string s = std::to_string(n);

        std::stringstream err;
        execore::DiagnosticEngine diag(err, false);
        execore::Lexer lexer(s, "prop.exe", diag);

        auto tok = lexer.next_token();
        assert(tok.kind() == execore::TokenKind::IntLiteral);
        assert(tok.lexeme() == s);
        int64_t parsed = std::stoll(std::string(tok.lexeme()));
        assert(parsed == n);
    }
    std::cout << "[PASS] Property 3: 500 trials of integer lexer token round-trips verified.\n";
}

void test_property_truthiness_laws() {
    assert(!execore::Value().is_truthy());             // None is falsy
    assert(!execore::Value(0).is_truthy());            // 0 is falsy
    assert(execore::Value(1).is_truthy());             // 1 is truthy
    assert(execore::Value(-1).is_truthy());            // -1 is truthy
    assert(!execore::Value(std::string("")).is_truthy()); // "" is falsy
    assert(execore::Value(std::string("a")).is_truthy()); // "a" is truthy
    assert(!execore::Value::make_list({}).is_truthy());   // [] is falsy
    assert(execore::Value::make_list({execore::Value(0)}).is_truthy()); // [0] is truthy (len > 0)
    std::cout << "[PASS] Property 4: Truthiness algebraic invariants verified.\n";
}

int main() {
    std::cout << "=== Running Property-Based Tests (Frontier C++2026 Milestone 3) ===\n";
    test_property_arithmetic_invariants();
    test_property_string_slicing_roundtrip();
    test_property_token_roundtrip();
    test_property_truthiness_laws();
    std::cout << "=== All Property-Based Tests Passed Successfully! ===\n";
    return 0;
}
