cmake_minimum_required(VERSION 3.13)

set(NAME rpi-pico-evse)
set(URL "https://github.com/dcooperdalrymple/rpi-pico-evse")

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)

# pico_sdk_import.cmake is a single file copied from this SDK
# note: this must happen before project()
include(pico_sdk_import.cmake)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# rest of your project
project(${NAME} C CXX ASM)

add_executable(${NAME}
    src/display.cpp
    src/rotary.cpp
    src/leds.cpp
    src/pilot.cpp
    main.cpp
)
add_subdirectory(./lib/pico-ss-oled)

pico_generate_pio_header(${NAME} ${CMAKE_CURRENT_LIST_DIR}/src/rotary.pio)

pico_add_extra_outputs(${NAME})
target_link_libraries(${NAME} pico_stdlib hardware_gpio hardware_pio hardware_pwm pico_ss_oled pico_double)

pico_set_program_url(${NAME} ${URL})
