#!/usr/bin/env bash

set -eou pipefail

PRESET="${1:-debug}"

echo "[INFO]: Removing previous build directory"
rm -rf ./build > /dev/null 2>&1
echo "[DONE]"

echo "[INFO]: Cmake Configure -- ${PRESET}"
cmake --preset "$PRESET" 
echo "[DONE]"

echo "[INFO]: Cmake build -- ${PRESET}"
cmake --build --preset "$PRESET"
echo "[DONE]"



