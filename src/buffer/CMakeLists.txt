add_library(minidb_buffer
    lru_replacer.cpp
    buffer_pool_manager.cpp
)

target_include_directories(minidb_buffer
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(minidb_buffer
    PUBLIC
        minidb
        minidb_storage
        minidb_disk
)
