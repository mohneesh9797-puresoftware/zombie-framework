#!/bin/bash

set -e
pushd ~
git -C emsdk pull || git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh
popd

