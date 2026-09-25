#include "execore/frontend/lexer.hpp"
#include "execore/frontend/parser.hpp"
#include "execore/semantics/symbol_table.hpp"
#include "execore/semantics/semantic_analyzer.hpp"
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
    execore::Lexer lexer(source, "fuzz_parser.exe", diag, 4);
    execore::Parser parser(lexer, diag);

    auto program = parser.parse_program();
    if (program && !diag.has_errors()) {
        execore::SymbolTable symbols;
        execore::SemanticAnalyzer analyzer(symbols, diag);
        analyzer.analyze(*program);
    }

    return 0;
}
