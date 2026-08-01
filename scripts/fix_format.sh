#!/usr/bin/env bash

set -e

clang-format -i -style=file \
    $(find src include test tools -type f \( -name "*.cpp" -o -name "*.h" \))
