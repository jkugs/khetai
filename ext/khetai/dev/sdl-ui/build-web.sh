#!/usr/bin/env bash
# Builds the WebAssembly game and assembles the static site in dist/ for Vercel.
# Vercel can't run emscripten, so deploy the prebuilt output (see README).
set -euo pipefail
cd "$(dirname "$0")"

EMSDK_ENV="${EMSDK_ENV:-$HOME/workspace/emsdk/emsdk_env.sh}"
source "$EMSDK_ENV" >/dev/null 2>&1

[ -f build-wasm/CMakeCache.txt ] || emcmake cmake -S . -B build-wasm -DSDL3_DIR="$PWD/SDL3-wasm/build-wasm"
emmake make -C build-wasm -j4

rm -rf dist
mkdir -p dist
cp web/* dist/
cp build-wasm/khet-sdl.js build-wasm/khet-sdl.wasm dist/
echo "built dist/: $(ls dist | tr '\n' ' ')"
