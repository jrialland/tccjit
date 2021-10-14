#!/bin/bash
git submodule update
mkdir -p build/Release
cd build/Release
CMAKE_BUILD_TYPE=Release cmake ../..
cmake --build .
./tests
