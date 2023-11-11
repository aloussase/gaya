# Installing Gaya

## Dependencies

You need to have a C++ compiler and CMake. You can install them on Debian by
running the following command:

```
sudo apt-get install build-essential cmake
```

## Building Gaya

You can install Gaya by building it from source:

```
git clone --recurse-submodules https://github.com/aloussase/gaya
cd gaya
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

The process should be straightforward, I do it always when working on the
language ;).
