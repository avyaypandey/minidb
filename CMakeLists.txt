cmake_minimum_required(VERSION 3.20)

project(
    MiniDB
    VERSION 0.1.0
    DESCRIPTION "A lightweight educational DBMS"
    LANGUAGES CXX
)

# -----------------------------------------------------------------------------
# C++ Standard
# -----------------------------------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------------------------------------------------------
# Compiler Warnings
# -----------------------------------------------------------------------------

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )
endif()

# -----------------------------------------------------------------------------
# Testing
# -----------------------------------------------------------------------------

enable_testing()

# -----------------------------------------------------------------------------
# Subdirectories
# -----------------------------------------------------------------------------

add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(test)
