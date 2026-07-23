#!/usr/bin/env bash

set -e

clang-format --dry-run --Werror \
    $(find src include test tools -type f \( -name "*.cpp" -o -name "*.h" \))
