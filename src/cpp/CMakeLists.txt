# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.16)

project(cktap_protocol VERSION 0.0.1 LANGUAGES CXX)

add_library(cktap_protocol SHARED
    "${PROJECT_SOURCE_DIR}/enums.cpp"
    "${PROJECT_SOURCE_DIR}/exports.cpp"
    "${PROJECT_SOURCE_DIR}/internal/card_operation.cpp"
    "${PROJECT_SOURCE_DIR}/internal/exceptions.cpp"
    "${PROJECT_SOURCE_DIR}/internal/globals.cpp"
    "${PROJECT_SOURCE_DIR}/internal/tap_protocol_thread.cpp"
    "${PROJECT_SOURCE_DIR}/internal/utils.cpp")

target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/")

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "cktap_protocol")

target_compile_definitions(${PROJECT_NAME} PUBLIC DART_SHARED_LIB)
target_compile_definitions(cktap_protocol PRIVATE "CKTAP_PLATFORM_${CMAKE_SYSTEM_NAME}=1")

# Flutter by default doesn't initialize submodules for plugins. We must make
# sure tap-protocol is available
if("${CMAKE_HOST_SYSTEM_NAME}" STREQUAL "WINDOWS")
    # TODO: Add Windows command
else()
    execute_process(COMMAND "${PROJECT_SOURCE_DIR}/../../scripts/update_submodules.sh")
endif()

# Link against Nunchuk's tap-protocol library so we can interface with it
add_subdirectory("${PROJECT_SOURCE_DIR}/../../contrib/tap-protocol"
                 "${PROJECT_SOURCE_DIR}/../../build/tap-protocol")
target_compile_options(tap-protocol PRIVATE "-w")
target_link_libraries(${PROJECT_NAME} PUBLIC tap-protocol)
