cmake_minimum_required(VERSION 3.20)
project(kvstore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Core logic as a library so both the app and the tests can use it
add_library(kvstore_lib src/kv_store.cpp src/command.cpp src/wal.cpp src/database.cpp)
target_include_directories(kvstore_lib PUBLIC src)

# The REPL program
add_executable(kvstore src/main.cpp)
target_link_libraries(kvstore PRIVATE kvstore_lib)

# GoogleTest, downloaded automatically on first configure
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)  # needed on Windows
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(kvstore_tests
  tests/kv_store_test.cpp
  tests/command_test.cpp
  tests/wal_test.cpp
  tests/database_test.cpp
)
target_link_libraries(kvstore_tests PRIVATE kvstore_lib GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(kvstore_tests)