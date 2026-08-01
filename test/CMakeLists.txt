include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

FetchContent_MakeAvailable(googletest)


add_executable(minidb_tests

    hello_test.cpp

    common/result_test.cpp
    common/status_test.cpp

    storage/page_test.cpp
    storage/tuple_test.cpp
    storage/serializer_test.cpp

    disk/disk_manager_test.cpp

    buffer/lru_replacer_test.cpp
    buffer/buffer_pool_manager_test.cpp

    integration/page_disk_test.cpp
    integration/persistence_test.cpp
    integration/buffer_disk_test.cpp
    integration/storage_integration_test.cpp
)


target_link_libraries(minidb_tests
    PRIVATE
        minidb
        minidb_storage
        minidb_disk
        minidb_buffer
        GTest::gtest_main
)


include(GoogleTest)

gtest_discover_tests(minidb_tests)
