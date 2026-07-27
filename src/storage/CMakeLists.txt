add_library(minidb_storage
    page.cpp
    tuple.cpp
    serializer.cpp
)

target_include_directories(minidb_storage PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
