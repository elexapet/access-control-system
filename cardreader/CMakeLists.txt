cmake_minimum_required(VERSION 3.11)

# Default type is release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project(acs-panel VERSION 1.0 LANGUAGES C)


set(CMAKE_EXECUTABLE_SUFFIX ".axf")

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}")

# LPCOpen chip library

add_library(lpc_chip_11cxx STATIC IMPORTED)

target_include_directories(lpc_chip_11cxx INTERFACE
    lib/include
)

set_target_properties(lpc_chip_11cxx
    PROPERTIES
    IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_LIST_DIR}/lib/Debug/liblpc_chip_11cxx.a"
    IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_LIST_DIR}/lib/Release/liblpc_chip_11cxx.a"
)


# Firmware

add_executable(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE
    app
    bsp
    bsp/board
    bsp/can
    bsp/weigand
    freertos/include
)

target_sources(${PROJECT_NAME} PRIVATE
    app/crp.c
    app/start.c
    app/static_cache.c
    app/terminal.c
    app/terminal_config.c
    bsp/brownout.c
    bsp/reader.c
    bsp/storage.c
    bsp/watchdog.c
    bsp/board/board.c
    bsp/board/board_sysinit.c
    bsp/board/cr_startup_lpc11xx.c
    bsp/board/sysinit.c
    bsp/can/can_term_driver.c
    bsp/weigand/weigand.c
    freertos/croutine.c
    freertos/event_groups.c
    freertos/heap_1.c
    freertos/list.c
    freertos/port.c
    freertos/queue.c
    freertos/stream_buffer.c
    freertos/tasks.c
    freertos/timers.c
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    CORE_M0
    __USE_LPCOPEN
    CR_INTEGER_PRINTF
    __LPC11XX__
    __NEWLIB__
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -std=c11
    -pedantic
    -Wall
    -Wextra
    -Wparentheses
    -fno-strict-aliasing
    -fno-common
    -fmessage-length=0
    -fno-builtin
    -ffunction-sections
    -fdata-sections
    -fsingle-precision-constant
    -fstack-usage
)

target_link_libraries(${PROJECT_NAME}
    lpc_chip_11cxx
)

if(${CMAKE_BUILD_TYPE} STREQUAL Release)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        RELEASE
        NDEBUG
        __CODE_RED
    )
    target_compile_options(${PROJECT_NAME} PRIVATE
        -O3
        -g
    )
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        DEBUG
    )
    target_compile_options(${PROJECT_NAME} PRIVATE
        -O0
        -g3
    )
endif()


linker_script_add(${PROJECT_NAME} "${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}_${CMAKE_BUILD_TYPE}.ld")
linker_script_target_dependency(${PROJECT_NAME} "${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}_${CMAKE_BUILD_TYPE}.ld")

# Post build steps
firmware_size(${PROJECT_NAME})
generate_object(${PROJECT_NAME} .bin binary)

if(${CMAKE_BUILD_TYPE} STREQUAL Release)
    calculate_checksum(${PROJECT_NAME})
endif()
