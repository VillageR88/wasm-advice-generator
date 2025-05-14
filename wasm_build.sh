#!/bin/bash

mkdir -p build
emcc -s USE_SDL=2 -s USE_SDL_TTF=2 -s USE_SDL_IMAGE=2 src/main.cpp -s FETCH=1  -s WASM=1 -o build/index.js --preload-file assets/fonts/ --preload-file assets/images/ --use-preload-plugins
cp -r template/* build/