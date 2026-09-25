#include <benchmark/benchmark.h>
#include "execore/frontend/source_manager.hpp"
#include "execore/frontend/lexer.hpp"
#include "execore/frontend/parser.hpp"
#include "execore/semantics/symbol_table.hpp"
#include "execore/semantics/semantic_analyzer.hpp"
#include "execore/runtime/interpreter.hpp"
#include <sstream>
#include <string>

namespace {

static void BM_InterpreterFibonacci(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    const std::string code =
        "def fib(x)\n"
        "    if x <= 1\n"
        "        return x\n"
        "    return fib(x - 1) + fib(x - 2)\n"
        "\n"
        "int res = fib(" + std::to_string(n) + ")\n"
        "return res\n";

    execore::SourceManager sm;
    auto src_opt = sm.add_source("fib.exe", code);
    std::ostringstream err_stream;
    execore::DiagnosticEngine diag(err_stream, false);

    execore::Lexer lexer(src_opt, "fib.exe", diag);
    execore::Parser parser(lexer, diag);
    auto program = parser.parse_program();

    execore::SymbolTable symbols;
    execore::SemanticAnalyzer analyzer(symbols, diag);
    analyzer.analyze(*program);

    for (auto _ : state) {
        std::istringstream in_stream;
        std::ostringstream out_stream;
        execore::Interpreter interpreter(diag, &sm, in_stream, out_stream);
        int res = interpreter.execute(*program);
        benchmark::DoNotOptimize(res);
    }
}

static void BM_InterpreterLoopAccumulate(benchmark::State& state) {
    const int iterations = static_cast<int>(state.range(0));
    const std::string code =
        "int sum = 0\n"
        "int i = 0\n"
        "while i < " + std::to_string(iterations) + "\n"
        "    sum += i\n"
        "    i += 1\n"
        "return sum\n";

    execore::SourceManager sm;
    auto src_opt = sm.add_source("loop.exe", code);
    std::ostringstream err_stream;
    execore::DiagnosticEngine diag(err_stream, false);

    execore::Lexer lexer(src_opt, "loop.exe", diag);
    execore::Parser parser(lexer, diag);
    auto program = parser.parse_program();

    execore::SymbolTable symbols;
    execore::SemanticAnalyzer analyzer(symbols, diag);
    analyzer.analyze(*program);

    for (auto _ : state) {
        std::istringstream in_stream;
        std::ostringstream out_stream;
        execore::Interpreter interpreter(diag, &sm, in_stream, out_stream);
        int res = interpreter.execute(*program);
        benchmark::DoNotOptimize(res);
    }

    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) * iterations);
}

} // namespace

BENCHMARK(BM_InterpreterFibonacci)->Arg(10)->Arg(15);
BENCHMARK(BM_InterpreterLoopAccumulate)->RangeMultiplier(10)->Range(100, 10000);
