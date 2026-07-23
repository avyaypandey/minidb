add_executable(minidb_cli
    minidb_cli.cpp
)

target_link_libraries(minidb_cli
    PRIVATE
        minidb
)
