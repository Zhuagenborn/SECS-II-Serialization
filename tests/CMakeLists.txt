set(TEST_NAME ${LIB_NAME}_tests)

include(GoogleTest)

add_executable(${TEST_NAME})

target_sources(${TEST_NAME}
    PRIVATE
        ${TEST_NAME}.cpp
)

target_link_libraries(${TEST_NAME}
    PRIVATE
        ${GTEST_LIB}
        ${LIB_NAME}
        bit_manip
)

gtest_discover_tests(${TEST_NAME})

# ---------- performance tests ----------
set(PERF_TEST_NAME ${LIB_NAME}_performance_tests)

add_executable(${PERF_TEST_NAME})

target_sources(${PERF_TEST_NAME}
    PRIVATE
        performace-test.cpp
)

target_link_libraries(${PERF_TEST_NAME}
    PRIVATE
        ${GTEST_LIB}
        ${LIB_NAME}
        ${TARGET_NAME}
        bit_manip
)

gtest_discover_tests(${PERF_TEST_NAME})
