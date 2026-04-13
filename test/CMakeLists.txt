set(TEST_BINARY onboarding-tests)

set(TEST_DEPENDENCIES
    ${CMAKE_SOURCE_DIR}/lm75bd/lm75bd.c
)

set(TEST_SOURCES
    ${CMAKE_SOURCE_DIR}/test/main.cpp
    ${CMAKE_SOURCE_DIR}/test/test_driver.cpp
    ${CMAKE_SOURCE_DIR}/test/mock_logging.c
)

set(TEST_SOURCES ${TEST_SOURCES} ${TEST_DEPENDENCIES})

add_executable(${TEST_BINARY} ${TEST_SOURCES})

target_include_directories(${TEST_BINARY}
    PRIVATE
    fff
    ${CMAKE_SOURCE_DIR}/lm75bd
    ${CMAKE_SOURCE_DIR}/sys
    ${CMAKE_SOURCE_DIR}/sys/i2c
    ${CMAKE_SOURCE_DIR}/sys/console_io
    ${CMAKE_SOURCE_DIR}/sys/logging
)

target_link_libraries(${TEST_BINARY} PRIVATE
    GTest::GTest
)

add_test(${TEST_BINARY} ${TEST_BINARY})