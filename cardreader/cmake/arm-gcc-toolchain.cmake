cmake_minimum_required(VERSION 3.11)

#---------------------
# ARM GCC Toolchain for cross-compiling to LPC11C24_301
#---------------------

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL Windows)
    set(COMPILER_SEARCH_CMD where)
    set(NINJA "${CMAKE_CURRENT_LIST_DIR}/utils/ninja.exe")
    set(CHECKSUM_BIN "${CMAKE_CURRENT_LIST_DIR}/utils/checksum.exe")
    set(EXE ".exe")
else()
    message(FATAL_ERROR "Unsupported host system")
endif()

set(CMAKE_MAKE_PROGRAM "${NINJA}" CACHE FILEPATH "")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(TOOLCHAIN_PREFIX arm-none-eabi-)

if(NOT DEFINED ARM_COMPILER_PATH)
    get_filename_component(ARM_COMPILER_PATH ${TOOLCHAIN_PREFIX}gcc${EXE} ABSOLUTE)
endif()

get_filename_component(ARM_TOOLCHAIN_DIR "${ARM_COMPILER_PATH}" DIRECTORY)
message(STATUS "Using ARM GCC toolchain:\"${ARM_TOOLCHAIN_DIR}\"")

set(CMAKE_C_COMPILER "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}gcc${EXE}")
set(CMAKE_ASM_COMPILER "${CMAKE_C_COMPILER}")
set(CMAKE_CXX_COMPILER "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}g++${EXE}")

set(CMAKE_OBJCOPY "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}objcopy${EXE}" CACHE INTERNAL "objcopy tool")
set(CMAKE_SIZE_UTIL "${ARM_TOOLCHAIN_DIR}/${TOOLCHAIN_PREFIX}size${EXE}" CACHE INTERNAL "size tool")

set(CMAKE_SYSROOT "${ARM_TOOLCHAIN_DIR}/../arm-none-eabi")
set(CMAKE_FIND_ROOT_PATH "${ARM_TOOLCHAIN_DIR}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

set(CMAKE_EXE_LINKER_FLAGS_INIT "-nostdlib -Xlinker --gc-sections -Xlinker -print-memory-usage -mcpu=cortex-m0 -mthumb")
set(CMAKE_C_FLAGS_INIT "-mcpu=cortex-m0 -mthumb")

#---------------------
# Helper functions
#---------------------

# Add custom command to print firmware size in Berkley format
function(firmware_size target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} -B
        "${CMAKE_CURRENT_BINARY_DIR}/${target}${CMAKE_EXECUTABLE_SUFFIX}"
    )
endfunction()

# Add a command to generate firmare in a provided format
function(generate_object target suffix type)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -v -O ${type}
        "${CMAKE_CURRENT_BINARY_DIR}/${target}${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_BINARY_DIR}/${target}${suffix}"
        COMMENT "Generating binary..."
    )
endfunction()

function(calculate_checksum target)
    if(CHECKSUM_BIN)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CHECKSUM_BIN} -p LPC11C24_301 -d "${CMAKE_CURRENT_BINARY_DIR}/${target}.bin"
            COMMENT "Calculating checksum..."
        )
    endif()
endfunction()

# Add custom linker script to the linker flags
function(linker_script_add target path_to_script)
    target_link_options(${target} PRIVATE -T "${path_to_script}")
endfunction()

# Update a target LINK_DEPENDS property with a custom linker script.
# That allows to rebuild that target if the linker script gets changed
function(linker_script_target_dependency target path_to_script)
    set_target_properties(${target} PROPERTIES LINK_DEPENDS "${path_to_script}")
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -Xlinker -Map=\"${target}.map\"")
endfunction()