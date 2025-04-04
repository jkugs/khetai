# Khet SDL3

## Overview

A Khet game built with SDL3. Uses KhetAI for moves.

---

## Controls

- **Mouse Click**: Select and move Silver pieces  
- **← / → Arrow Keys**: Rotate selected Silver piece
- **KhetAI**: Will automatically make a move after a Silver piece is moved
- **Spacebar**: Manually fire laser  
- **Enter**: Trigger an AI (Red) move

---

### Building & Running: Desktop

```bash
cmake -S . -B build
cmake --build build/
./build/khet-sdl
```

### Building & Running: WASM

```bash
source ~/path/to/emsdk/emsdk_env.sh
emcmake cmake -S . -B build-wasm -DSDL3_DIR=$PWD/SDL3-wasm/build-wasm
emmake make -C build-wasm -j4
emrun --no_browser --port 8080 build-wasm
```

Then open [http://localhost:8080](http://localhost:8080) in your browser.

---

### Setup SDL3 for WASM (one-time)

```bash
source ~/path/to/emsdk/emsdk_env.sh
git clone https://github.com/libsdl-org/SDL.git SDL3-wasm
cd SDL3-wasm
git checkout release-3.2.x
mkdir build-wasm && cd build-wasm
emcmake cmake .. -DSDL_TESTS=OFF
emmake make -j4
```
