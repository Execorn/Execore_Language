#!/usr/bin/env bash
# ==============================================================================
# Execore Frontier C++2026: Benchmark Suite Runner
# Runs microbenchmarks for Lexer, Parser, and Interpreter
# ==============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
BENCH_DIR="${ROOT_DIR}/build/bench/benchmarks"

if [ ! -d "${BENCH_DIR}" ]; then
    echo "Configuring and building benchmarks preset..."
    cmake --preset bench
    cmake --build --preset bench
fi

echo "================================================================================"
echo " Execore Performance Benchmark Suite (Google Benchmark)"
echo "================================================================================"

echo ""
echo "--- [1/3] Lexer Throughput Benchmark (MB/s) ---"
"${BENCH_DIR}/bench_lexer" --benchmark_min_time=0.1s

echo ""
echo "--- [2/3] Parser Expression & AST Throughput Benchmark ---"
"${BENCH_DIR}/bench_parser" --benchmark_min_time=0.1s

echo ""
echo "--- [3/3] Interpreter Recursion & Loop Accumulator Latency ---"
"${BENCH_DIR}/bench_interpreter" --benchmark_min_time=0.1s

echo ""
echo "================================================================================"
echo " All Benchmarks Executed Successfully!"
echo "================================================================================"
