include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(minidb_tests
    hello_test.cpp
)

target_link_libraries(minidb_tests
    PRIVATE
        minidb
        GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(minidb_tests)

