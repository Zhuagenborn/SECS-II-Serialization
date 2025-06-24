cmake_minimum_required(VERSION 3.20.0)

project(secs2 LANGUAGES CXX)

set(LIB_NAME ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

option(SECS2_BUILD_TESTS "Build unit tests for the SECS-II serialization library" OFF)
if(SECS2_BUILD_TESTS)
    find_package(GTest)
    if(GTest_FOUND)
        enable_testing()
        set(GTEST_LIBS GTest::gtest_main)
        set(GMOCK_LIBS GTest::gmock_main)
        set(CMAKE_GTEST_DISCOVER_TESTS_DISCOVERY_MODE PRE_TEST)
        add_subdirectory(tests)
    endif()
endif()

add_subdirectory(src)
add_subdirectory(external)