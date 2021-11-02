#!/bin/sh

platform="$(uname)"

if [ "$platform" = "Darwin" ]; then
    if command -v brew > /dev/null; then
        CMAKE_PREFIX_PATH="$(brew --prefix qt\@5)"
        export CMAKE_PREFIX_PATH
    fi
fi

mkdir -p build
cd build || exit

cmake -GNinja -DCMAKE_BUILD_TYPE=Debug ..
