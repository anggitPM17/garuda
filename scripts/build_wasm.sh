#!/usr/bin/env bash
# =====================================================================
# GARUDA BEAM — EMSCRIPTEN WASM BUILD SCRIPT
# =====================================================================
set -e

echo "======================================================="
echo "  GARUDA BEAM — WEBASSEMBLY (WASM) BUILD"
echo "======================================================="

mkdir -p build-wasm
cd build-wasm

emcmake cmake ../wasm -DCMAKE_BUILD_TYPE=Release
emmake make -j$(nproc 2>/dev/null || echo 2)

echo "======================================================="
echo "  WASM BUILD COMPLETE: site/wasm/garuda.js & garuda.wasm"
echo "======================================================="
