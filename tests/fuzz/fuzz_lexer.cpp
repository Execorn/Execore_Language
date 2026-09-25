#include "execore/frontend/lexer.hpp"
#include "execore/diagnostics/diagnostic_engine.hpp"
#include <cstdint>
#include <cstddef>
#include <string_view>
#include <sstream>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    if (size == 0) return 0;

    std::string_view source(reinterpret_cast<const char*>(data), size);
    std::stringstream err_stream;
    execore::DiagnosticEngine diag(err_stream, false);
    execore::Lexer lexer(source, "fuzz_input.exe", diag, 4);

    // Consume all tokens up to EOF or limit
    size_t token_count = 0;
    constexpr size_t max_tokens = 50000;
    while (token_count++ < max_tokens) {
        auto token = lexer.next_token();
        if (token.is(execore::TokenKind::Eof)) {
            break;
        }
    }

    return 0;
}
