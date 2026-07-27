add_library(minidb INTERFACE)

target_include_directories(minidb
    INTERFACE
        ${PROJECT_SOURCE_DIR}/include
)

add_subdirectory(storage)
