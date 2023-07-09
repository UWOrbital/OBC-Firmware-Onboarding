cmake_minimum_required(VERSION 3.15)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/toolchain_gcc.cmake)

project(onboarding)

enable_language(C CXX)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(SOURCES
    main.c
    sys/console_io/console.c
    sys/i2c/i2c_io.c
    services/controller/controller.c
    
    lm75bd/lm75bd.c
    services/thermal_mgr/thermal_mgr.c
)

add_executable(onboarding ${SOURCES})

add_subdirectory(freertos)

target_include_directories(onboarding PRIVATE
    sys
    sys/i2c
    sys/console_io
    lm75bd
    services/controller
    services/thermal_mgr
)

target_link_libraries(onboarding PRIVATE
    FreeRTOS
    Threads::Threads
)
