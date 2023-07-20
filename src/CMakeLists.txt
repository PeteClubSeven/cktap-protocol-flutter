# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.16)

project(cktap_protocol VERSION 0.0.1 LANGUAGES CXX)

add_library(cktap_protocol SHARED
  "${PROJECT_SOURCE_DIR}/Exports.cpp"
  "${PROJECT_SOURCE_DIR}/Enums.cpp"
  "${PROJECT_SOURCE_DIR}/Internal/TapProtocolThread.cpp"
  "${PROJECT_SOURCE_DIR}/Internal/Utils.cpp"
)

target_include_directories(${PROJECT_NAME}
  PUBLIC "${PROJECT_SOURCE_DIR}/"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
  OUTPUT_NAME "cktap_protocol"
)

target_compile_definitions(${PROJECT_NAME} PUBLIC DART_SHARED_LIB)

# Ensure libsecp256k1 is built for the Nunchuk tap-protocol library to link
if (ANDROID)
  execute_process(COMMAND "${PROJECT_SOURCE_DIR}/../android/build_libsecp256k1.sh" "${CMAKE_ANDROID_NDK}" "${ANDROID_ABI}")
endif()

# Link against Nunchuks tap-protocol library so we can interface with it
add_subdirectory("${PROJECT_SOURCE_DIR}/../contrib/tap-protocol" "${PROJECT_SOURCE_DIR}/../build/tap-protocol")
target_compile_options(tap-protocol PRIVATE "-w")
target_link_libraries(${PROJECT_NAME} PUBLIC tap-protocol)
