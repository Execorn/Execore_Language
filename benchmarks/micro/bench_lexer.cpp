#include <benchmark/benchmark.h>
#include "execore/frontend/source_manager.hpp"
#include "execore/frontend/lexer.hpp"
#include "execore/diagnostics/diagnostic_engine.hpp"
#include <sstream>
#include <string>

namespace {

std::string generate_synthetic_script(size_t lines) {
    std::string script;
    script.reserve(lines * 64);
    for (size_t i = 0; i < lines; ++i) {
        script += "def compute_val_" + std::to_string(i) + "(a, b)\n";
        script += "    int temp = a * 2 + b * 3\n";
        script += "    if temp > 100\n";
        script += "        return temp - 50\n";
        script += "    return temp + 50\n";
    }
    return script;
}

static void BM_LexerThroughput(benchmark::State& state) {
    const size_t line_count = static_cast<size_t>(state.range(0));
    const std::string script = generate_synthetic_script(line_count);

    execore::SourceManager sm;
    auto src_opt = sm.add_source("bench_synthetic.exe", script);
    std::ostringstream err_stream;
    execore::DiagnosticEngine diag(err_stream, false);

    for (auto _ : state) {
        execore::Lexer lexer(src_opt, "bench_synthetic.exe", diag);
        size_t token_count = 0;
        while (true) {
            auto tok = lexer.next_token();
            benchmark::DoNotOptimize(tok);
            ++token_count;
            if (tok.is(execore::TokenKind::Eof)) {
                break;
            }
        }
        benchmark::DoNotOptimize(token_count);
    }

    state.SetBytesProcessed(static_cast<int64_t>(state.iterations()) *
                            static_cast<int64_t>(script.size()));
    state.SetItemsProcessed(static_cast<int64_t>(state.iterations()) *
                            static_cast<int64_t>(line_count * 5));
}

} // namespace

BENCHMARK(BM_LexerThroughput)->RangeMultiplier(4)->Range(16, 1024);
