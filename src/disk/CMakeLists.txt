add_library(minidb_disk
    disk_manager.cpp
)

target_include_directories(minidb_disk
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(minidb_disk
    PUBLIC
        minidb
)
