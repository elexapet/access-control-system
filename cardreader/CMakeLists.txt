cmake_minimum_required(VERSION 3.11)

# Default type is release
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

project(acs-panel VERSION 1.0 LANGUAGES C)


set(CMAKE_EXECUTABLE_SUFFIX ".axf")

set(PROJECT_ROOT "${CMAKE_CURRENT_LIST_DIR}")

set(CMAKE_INSTALL_PREFIX "${PROJECT_ROOT}")

# LPCOpen chip library

add_library(lpc_chip_11cxx STATIC IMPORTED)

target_include_directories(lpc_chip_11cxx INTERFACE
    "${PROJECT_ROOT}/lib/include"
)

set_target_properties(lpc_chip_11cxx
    PROPERTIES
    IMPORTED_LOCATION_DEBUG "${PROJECT_ROOT}/lib/Debug/liblpc_chip_11cxx.a"
    IMPORTED_LOCATION_RELEASE "${PROJECT_ROOT}/lib/Release/liblpc_chip_11cxx.a"
)


# Firmware

add_executable(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${PROJECT_ROOT}/app"
    "${PROJECT_ROOT}/bsp"
    "${PROJECT_ROOT}/bsp/board"
    "${PROJECT_ROOT}/bsp/can"
    "${PROJECT_ROOT}/bsp/weigand"
    "${PROJECT_ROOT}/freertos/include"
)

target_sources(${PROJECT_NAME} PRIVATE
    "${PROJECT_ROOT}/app/crp.c"
    "${PROJECT_ROOT}/app/start.c"
    "${PROJECT_ROOT}/app/static_cache.c"
    "${PROJECT_ROOT}/app/terminal.c"
    "${PROJECT_ROOT}/app/terminal_config.c"
    "${PROJECT_ROOT}/bsp/brownout.c"
    "${PROJECT_ROOT}/bsp/reader.c"
    "${PROJECT_ROOT}/bsp/storage.c"
    "${PROJECT_ROOT}/bsp/watchdog.c"
    "${PROJECT_ROOT}/bsp/board/board.c"
    "${PROJECT_ROOT}/bsp/board/board_sysinit.c"
    "${PROJECT_ROOT}/bsp/board/cr_startup_lpc11xx.c"
    "${PROJECT_ROOT}/bsp/board/sysinit.c"
    "${PROJECT_ROOT}/bsp/can/can_term_driver.c"
    "${PROJECT_ROOT}/bsp/weigand/weigand.c"
    "${PROJECT_ROOT}/freertos/croutine.c"
    "${PROJECT_ROOT}/freertos/event_groups.c"
    "${PROJECT_ROOT}/freertos/heap_1.c"
    "${PROJECT_ROOT}/freertos/list.c"
    "${PROJECT_ROOT}/freertos/port.c"
    "${PROJECT_ROOT}/freertos/queue.c"
    "${PROJECT_ROOT}/freertos/stream_buffer.c"
    "${PROJECT_ROOT}/freertos/tasks.c"
    "${PROJECT_ROOT}/freertos/timers.c"
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


linker_script_add(${PROJECT_NAME} "${PROJECT_ROOT}/cmake/${PROJECT_NAME}_${CMAKE_BUILD_TYPE}.ld")
linker_script_target_dependency(${PROJECT_NAME} "${PROJECT_ROOT}/cmake/${PROJECT_NAME}_${CMAKE_BUILD_TYPE}.ld")

# Post build steps
firmware_size(${PROJECT_NAME})
# generate BIN from AXF
generate_object(${PROJECT_NAME} ${CMAKE_EXECUTABLE_SUFFIX} .bin binary)
calculate_bin_checksum(${PROJECT_NAME})
# generate HEX from BIN with checksum
generate_object(${PROJECT_NAME} ${CMAKE_EXECUTABLE_SUFFIX} .hex ihex)
