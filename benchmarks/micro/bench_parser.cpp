#include <benchmark/benchmark.h>
#include "execore/frontend/source_manager.hpp"
#include "execore/frontend/lexer.hpp"
#include "execore/frontend/parser.hpp"
#include "execore/diagnostics/diagnostic_engine.hpp"
#include <sstream>
#include <string>

namespace {

std::string generate_math_expressions(size_t count) {
    std::string script;
    script.reserve(count * 64);
    for (size_t i = 0; i < count; ++i) {
        script += "int v_" + std::to_string(i) + " = 10 + 20 * 30 - 40 / 5 + (" + std::to_string(i) + " * 7)\n";
    }
    return script;
}

static void BM_ParserExpressionThroughput(benchmark::State& state) {
    const size_t expr_count = static_cast<size_t>(state.range(0));
    const std::string script = generate_math_expressions(expr_count);

    execore::SourceManager sm;
    auto src_opt = sm.add_source("bench_expr.exe", script);
    std::ostringstream err_stream;
    execore::DiagnosticEngine diag(err_stream, false);

    for (auto _ : state) {
        execore::Lexer lexer(src_opt, "bench_expr.exe", diag);
        execore::Parser parser(lexer, diag);
        auto program = parser.parse_program();
        benchmark::DoNotOptimize(program);
    }

    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) *
                            static_cast<int64_t>(script.size()));
    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) *
                            static_cast<int64_t>(expr_count));
}

} // namespace

BENCHMARK(BM_ParserExpressionThroughput)->RangeMultiplier(4)->Range(16, 512);
