#!/usr/bin/env bash
# =====================================================================
# GARUDA BEAM — LINUX BUILD SCRIPT
# Kompilasi Garuda BEAM menggunakan CMake & GCC/Clang
# =====================================================================
set -e

BUILD_DIR="build"
BUILD_TYPE="Release"

echo "======================================================="
echo "  GARUDA BEAM — LINUX COMPILATION SCRIPT"
echo "======================================================="

# Create build directory if needed
mkdir -p "${BUILD_DIR}"

echo "[1/2] Configuring CMake for Linux (${BUILD_TYPE})..."
cmake -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE="${BUILD_TYPE}"

echo "[2/2] Building all targets with $(nproc) parallel jobs..."
cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}" -j"$(nproc)"

echo "======================================================="
echo "  BUILD SUCCESSFUL! Executables in ${BUILD_DIR}/"
echo "======================================================="
