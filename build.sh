#!/bin/bash
set -e 

CXX="clang++"
STD="-std=c++20"
INC="-I./include -isystem ./include/lib"
WARNINGS="-Wall -Wextra -Wconversion -Wsign-conversion -Wpedantic -Werror -fno-exceptions"
LINKER_SPECIFC=""

MODE=${1:-debug} 

if [ "$MODE" = "release" ]; then
    echo "=== Building in RELEASE mode ==="
    OPT_FLAGS="-O3 -flto -DNDEBUG"
else
    echo "=== Building in DEBUG mode ==="
    OPT_FLAGS="-g -O0" 
fi

mkdir -p build/lib
mkdir -p build/bin

echo "[1/3] Compiling libraries..."
for src in src/lib/*.cpp; do
    [ -e "$src" ] || continue 
    
    obj="build/lib/$(basename "${src%.cpp}.o")"
    
    if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ]; then
        echo "  -> Compiling $src"
        $CXX $STD $OPT_FLAGS $INC -c "$src" -o "$obj"
    fi
done

echo "[2/3] Compiling ShadowHTTP..."
for src in src/*.cpp; do
    [ -e "$src" ] || continue
    
    obj="build/$(basename "${src%.cpp}.o")"
    
    if [ ! -f "$obj" ] || [ "$src" -nt "$obj" ]; then
        echo "  -> Compiling $src"
        $CXX $STD $OPT_FLAGS $WARNINGS $INC -c "$src" -o "$obj"
    fi
done

echo "[3/3] Linking..."
$CXX $OPT_FLAGS $LINKER_SPECIFC build/*.o build/lib/*.o -o build/bin/server

echo "Done! Server compiled in $MODE mode."