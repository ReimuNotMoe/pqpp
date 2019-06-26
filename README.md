# pqpp

A simple and easy-to-use async C++ PostgreSQL client library.

## Features
- Easy to use
- No templates and complicated data structures
- The usage is almost identical with nodejs's [pg](https://node-postgres.com/)

## Usage
See [demo.cpp](https://github.com/ReimuNotMoe/pqpp/blob/master/demo.cpp)

## Caveats
The project is in alpha & WIP state. It's not recommended to use in production environments.

## Build
### Dependencies
- C++17 compatible compiler and runtime
- libpg
- OpenSSL
- boost_system
- boost_coroutine

### Compile
Nearly all my projects use CMake. It's very simple:

    mkdir build
    cd build
    cmake ..
    make -j `nproc`

