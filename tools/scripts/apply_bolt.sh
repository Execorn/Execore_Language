#!/usr/bin/env bash
# ==============================================================================
# Execore Frontier C++2026: Binary Optimization and Layout Tool (BOLT) Pipeline
# ==============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"

INPUT_BINARY="${1:-${ROOT_DIR}/build/release-pgo/execore}"
OUTPUT_BINARY="${2:-${ROOT_DIR}/build/release-pgo/execore.bolt}"

echo "================================================================================"
echo " Execore BOLT (Binary Optimization and Layout Tool) Optimizer"
echo " Target Input:  ${INPUT_BINARY}"
echo " Target Output: ${OUTPUT_BINARY}"
echo "================================================================================"

if [ ! -f "${INPUT_BINARY}" ]; then
    echo "ERROR: Input binary '${INPUT_BINARY}' does not exist. Please build with release-pgo first."
    exit 1
fi

BOLT_BIN="$(command -v llvm-bolt 2>/dev/null || command -v bolt 2>/dev/null || true)"
PERF2BOLT_BIN="$(command -v perf2bolt 2>/dev/null || true)"

if [ -z "${BOLT_BIN}" ]; then
    echo "⚠️  'llvm-bolt' was not detected in PATH."
    echo "   BOLT is an advanced LLVM post-link binary layout optimizer."
    echo "   To enable BOLT optimizations:"
    echo "     1. Install LLVM with BOLT enabled ('apt-get install -y llvm-19-tools' or build LLVM with -DLLVM_ENABLE_PROJECTS='bolt')."
    echo "     2. Rebuild Execore with '-Wl,--emit-relocs' enabled."
    echo "     3. Run this script again."
    echo "   Current binary remains fully optimized via ThinLTO and PGO."
    exit 0
fi

echo "  -> Found BOLT optimizer: ${BOLT_BIN}"

# Check for relocations in the binary
if ! readelf -r "${INPUT_BINARY}" | grep -q "Relocation section"; then
    echo "⚠️  Binary lacks relocations (--emit-relocs). Creating relocatable build..."
    RELOC_BUILD_DIR="${ROOT_DIR}/build/bolt-stage"
    cmake -B "${RELOC_BUILD_DIR}" -S "${ROOT_DIR}" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER=clang++ \
        -DCMAKE_C_COMPILER=clang \
        -DCMAKE_EXE_LINKER_FLAGS="-Wl,--emit-relocs"
    cmake --build "${RELOC_BUILD_DIR}" --target execore
    INPUT_BINARY="${RELOC_BUILD_DIR}/execore"
fi

# Instrument or optimize
echo "  -> Instrumenting binary for profiling..."
INSTR_BINARY="${INPUT_BINARY}.inst"
"${BOLT_BIN}" "${INPUT_BINARY}" -instrument --instrumentation-file="/tmp/execore_bolt.fdata" -o "${INSTR_BINARY}"

echo "  -> Running training benchmark through instrumented binary..."
for script in "${ROOT_DIR}"/examples/*/*.exe "${ROOT_DIR}"/examples/*.exe; do
    if [ -f "${script}" ]; then
        "${INSTR_BINARY}" "${script}" > /dev/null 2>&1 || true
    fi
done

echo "  -> Applying BOLT profile-guided binary layout optimizations..."
"${BOLT_BIN}" "${INPUT_BINARY}" \
    -data="/tmp/execore_bolt.fdata" \
    -o "${OUTPUT_BINARY}" \
    -reorder-blocks=ext-tsp \
    -reorder-functions=cdsort \
    -split-functions \
    -split-all-cold \
    -dyno-stats

rm -f "/tmp/execore_bolt.fdata" "${INSTR_BINARY}"
echo "================================================================================"
echo " BOLT Optimization Completed Successfully!"
echo " Result: ${OUTPUT_BINARY}"
echo "================================================================================"
ls -lh "${OUTPUT_BINARY}"
