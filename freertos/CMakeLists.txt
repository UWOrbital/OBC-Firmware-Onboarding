cmake_minimum_required(VERSION 3.15)

set(SOURCES
    source/os_croutine.c
    source/os_event_groups.c
    source/os_heap.c
    source/os_list.c
    source/os_port.c
    source/os_queue.c
    source/os_required_fns.c
    source/os_stream_buffer.c
    source/os_tasks.c
    source/os_timers.c
    source/os_wait_for_event.c
)

add_library(FreeRTOS STATIC ${SOURCES})

target_include_directories(FreeRTOS PUBLIC include)
