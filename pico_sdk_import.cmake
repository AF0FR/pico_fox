# Standard Raspberry Pi Pico SDK import helper.
if (DEFINED ENV{PICO_SDK_PATH} AND (NOT PICO_SDK_PATH))
    set(PICO_SDK_PATH $ENV{PICO_SDK_PATH})
endif()

if (NOT PICO_SDK_PATH)
    set(PICO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/pico-sdk")
endif()

get_filename_component(PICO_SDK_PATH "${PICO_SDK_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")

if (NOT EXISTS ${PICO_SDK_PATH})
    message(FATAL_ERROR
        "Pico SDK not found at '${PICO_SDK_PATH}'. Set PICO_SDK_PATH or place the SDK in ./pico-sdk.")
endif()

set(PICO_SDK_INIT_CMAKE_FILE ${PICO_SDK_PATH}/pico_sdk_init.cmake)
if (NOT EXISTS ${PICO_SDK_INIT_CMAKE_FILE})
    message(FATAL_ERROR "${PICO_SDK_INIT_CMAKE_FILE} not found")
endif()

include(${PICO_SDK_INIT_CMAKE_FILE})
