add_library(minidb INTERFACE)

target_include_directories(minidb
    INTERFACE
        ${PROJECT_SOURCE_DIR}/include
)

add_subdirectory(storage)
add_subdirectory(disk)
add_subdirectory(buffer)
