#!/bin/sh

cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -GNinja &&
cmake --build build &&
sudo cmake --install build
