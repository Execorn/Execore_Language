#!/usr/bin/env bash
# ==============================================================================
# Execore Frontier C++2026: Automated Profile-Guided Optimization (PGO) Pipeline
# ==============================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/../.." && pwd)"

GEN_BUILD_DIR="${ROOT_DIR}/build/pgo-generate"
USE_BUILD_DIR="${ROOT_DIR}/build/release-pgo"
PGO_PROF_DIR="${GEN_BUILD_DIR}/pgo_profiles"

# Clean previous profile build trees to guarantee hermetic profile consistency
rm -rf "${GEN_BUILD_DIR}" "${USE_BUILD_DIR}"

echo "================================================================================"
echo " [PGO Step 1/4] Configuring & Building Instrumentation Target"
echo "================================================================================"
cmake -B "${GEN_BUILD_DIR}" -S "${ROOT_DIR}" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_COMPILER=clang \
    -DEXECORE_PGO_MODE=GENERATE \
    -DEXECORE_ENABLE_THINLTO=ON \
    -DEXECORE_BUILD_TESTS=ON \
    -DEXECORE_BUILD_BENCHMARKS=OFF

cmake --build "${GEN_BUILD_DIR}" --target execore

echo "================================================================================"
echo " [PGO Step 2/4] Training Workloads Across Realistic Workloads & AST Trees"
echo "================================================================================"
mkdir -p "${PGO_PROF_DIR}"
export LLVM_PROFILE_FILE="${PGO_PROF_DIR}/code-%p.profraw"

EXECORE_BIN="${GEN_BUILD_DIR}/execore"

# Execute all sample scripts multiple iterations
for script in "${ROOT_DIR}"/examples/*/*.exe "${ROOT_DIR}"/examples/*.exe; do
    if [ -f "${script}" ]; then
        echo "  -> Training on $(basename "${script}")..."
        for _ in {1..20}; do
            "${EXECORE_BIN}" "${script}" > /dev/null 2>&1 || true
        done
    fi
done

# Train on synthetic high-depth AST scripts
echo "  -> Training on synthetic control flow and recursive workloads..."
python3 -c '
import subprocess, sys

script = """
def ackermann(m, n)
    if m == 0
        return n + 1
    if n == 0
        return ackermann(m - 1, 1)
    return ackermann(m - 1, ackermann(m, n - 1))

int val = ackermann(2, 4)
print val
return val
"""
with open("/tmp/execore_pgo_train.exe", "w") as f:
    f.write(script)
'
"${EXECORE_BIN}" "/tmp/execore_pgo_train.exe" > /dev/null 2>&1 || true
rm -f "/tmp/execore_pgo_train.exe"

echo "================================================================================"
echo " [PGO Step 3/4] Indexing & Merging Profile Data with llvm-profdata"
echo "================================================================================"
MERGED_PROFDATA="${PGO_PROF_DIR}/merged.profdata"
llvm-profdata merge -output="${MERGED_PROFDATA}" "${PGO_PROF_DIR}"/*.profraw

echo "  -> Profile successfully merged at: ${MERGED_PROFDATA}"
ls -lh "${MERGED_PROFDATA}"

echo "================================================================================"
echo " [PGO Step 4/4] Building Optimized Production Binary with ThinLTO + PGO"
echo "================================================================================"
mkdir -p "${USE_BUILD_DIR}/pgo_profiles"
cp "${MERGED_PROFDATA}" "${USE_BUILD_DIR}/pgo_profiles/merged.profdata"

cmake -B "${USE_BUILD_DIR}" -S "${ROOT_DIR}" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_C_COMPILER=clang \
    -DEXECORE_PGO_MODE=USE \
    -DEXECORE_ENABLE_THINLTO=ON \
    -DEXECORE_ENABLE_HARDENING=ON \
    -DEXECORE_BUILD_TESTS=OFF

cmake --build "${USE_BUILD_DIR}" --target execore

echo "================================================================================"
echo " Execore PGO + ThinLTO Optimized Release Build Completed Successfully!"
echo " Binary Location: ${USE_BUILD_DIR}/execore"
echo "================================================================================"
size "${USE_BUILD_DIR}/execore"
