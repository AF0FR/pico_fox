cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(PicoFox C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(PicoFox
    src/main.c
    src/audio.c
    src/fox.c
    src/morse.c
    src/radio.c
)

target_include_directories(PicoFox PRIVATE src)

target_link_libraries(PicoFox PRIVATE
    pico_stdlib
    hardware_clocks
    hardware_pwm
)

pico_enable_stdio_usb(PicoFox 1)
pico_enable_stdio_uart(PicoFox 0)
pico_add_extra_outputs(PicoFox)
