set(TEST_NAME ${LIB_NAME}_tests)

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