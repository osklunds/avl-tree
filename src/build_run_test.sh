#!/bin/bash


# Convenience script for building, running and testing

build_dir="$(git rev-parse --show-toplevel)/build"
mkdir -p "$build_dir"
cd "$build_dir"

cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 .. && make -j8 && ./main "$@"
