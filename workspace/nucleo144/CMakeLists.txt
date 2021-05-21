cmake_minimum_required(VERSION 3.15.3)
project(SpeakerRecognition VERSION 1.0)

# Enable use of C, C++, and Assembly
enable_language(C CXX ASM)

# Set versions of langauges used
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)


set(PROJECT_INCLUDE_DIRECTORIES ${PROJECT_INCLUDE_DIRECTORIES}
        inc/
        HAL_Driver/Inc/
        CMSIS/device
        CMSIS/core
        Utilities/STM32L4xx_Nucleo_144/
)

include(Sources.cmake)

set(EXECUTABLE ${PROJECT_NAME}.out)

# Set output directory - must be done before adding executables, libraries, etc
# CMAKE_SOURCE_DIR is the top level of the current CMake source build tree
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/output)

add_executable(${EXECUTABLE} ${PROJECT_SOURCES})

target_compile_definitions(${EXECUTABLE} PRIVATE
        -DUSE_HAL_DRIVER
        -DSTM32L4R5xx
)

target_include_directories(${EXECUTABLE} PRIVATE ${PROJECT_INCLUDE_DIRECTORIES})

target_compile_options(${EXECUTABLE} PRIVATE
        -mcpu=cortex-m4 # assemble for CPU <cpu name>
        -mthumb # Generate code for Thumb state.
        -mfpu=fpv4-sp-d16 # Specify the name of the target floating point hardware/format.
        -mfloat-abi=hard # Specify if floating point hardware should be used. Hard specifies
            # full hardware floating-point support

        -fdata-sections # Place data items into their own section.
        -ffunction-sections # Place each function into its own section.

        -Wall # Enable most warning messages.

        $<$<CONFIG:Debug>:-Og>
)

target_link_options(${EXECUTABLE} PRIVATE
        -T${CMAKE_SOURCE_DIR}/LinkerScript.ld
        -mcpu=cortex-m4
        -mthumb
        -mfpu=fpv4-sp-d16
        -mfloat-abi=hard
        -specs=nano.specs
        -lc
        -lm
        -lnosys
        -Wl,-Map=${PROJECT_NAME}.map,--cref
        -Wl,--gc-sections
        -Wl,--print-memory-usage
)

# Print executable size
add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND arm-none-eabi-size ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXECUTABLE})

# Create hex file
add_custom_command(TARGET ${EXECUTABLE}
        POST_BUILD
        COMMAND arm-none-eabi-objcopy -O ihex ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXECUTABLE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.hex
        COMMAND arm-none-eabi-objcopy -O binary ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${EXECUTABLE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.bin)
