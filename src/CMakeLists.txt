# The Flutter tooling requires that developers have CMake 3.10 or later
# installed. You should not increase this version, as doing so will cause
# the plugin to fail to compile for some customers of the plugin.
cmake_minimum_required(VERSION 3.16)

project(cktap_protocol_library VERSION 0.0.1 LANGUAGES CXX)

add_library(cktap_protocol SHARED
  "./cktap_protocol.cpp"
  "exports/cktapcard.cpp"
  "exports/tapsigner.cpp"
  "exports/satscard.cpp"
  "internal/transport.cpp"
)

set_target_properties(cktap_protocol PROPERTIES
  PUBLIC_HEADER cktap_protocol.h
  OUTPUT_NAME "cktap_protocol"
)

target_compile_definitions(cktap_protocol PUBLIC DART_SHARED_LIB)

# Ensure libsecp256k1 is built for the Nunchuk tap-protocol library to link
if (ANDROID)
  execute_process(COMMAND "${CMAKE_SOURCE_DIR}/../android/build_libsecp256k1.sh" "${CMAKE_ANDROID_NDK}" "${ANDROID_ABI}")
endif()

# Link against Nunchuk's tap-protocol library so we can interface with it
add_subdirectory("${CMAKE_SOURCE_DIR}/../contrib/tap-protocol" "${CMAKE_SOURCE_DIR}/../build/tap-protocol")
target_compile_options(tap-protocol PRIVATE "-w")
target_link_libraries(cktap_protocol PUBLIC tap-protocol)

#add_custom_command(
#    OUTPUT ${CMAKE_SOURCE_DIR}/dependencies/library/lib.o
#    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/dependencies/library
#    COMMAND make
#)W

#add_custom_target( libscep256k1 ALL COMMAND ./cmake_requirements.sh "${CMAKE_ANDROID_NDK}")
#if (${CMAKE_ANDROID_ARCH} MATCHES "^(arm|mips|x86)$")
#set_target_properties(libsecp256k1 PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/src/secp256k1/build/android/${ANDROID_ABI}/lib/libsecp256k1.a)
#else()
#set_target_properties(libsecp256k1 PROPERTIES IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/src/secp256k1/build/android/${ANDROID_ABI}/lib64/libsecp256k1.a)
#endif

#add_custom_target(build_libsecp256k1 ALL "${CMAKE_SOURCE_DIR}/build_libsecp256k1.sh" ARGS "${CMAKE_ANDROID_NDK}" "${ANDROID_ABI}")
#if (${CMAKE_ANDROID_ARCH} MATCHES "^(arm|mips|x86)$")
#    add_custom_command(OUTPUT

#    )
#        add_custom_command(
#                TARGET cktap_protocol_plugin PRE_BUILD
#                COMMAND "${CMAKE_SOURCE_DIR}/build_libsecp256k1.sh" ARGS "${CMAKE_ANDROID_NDK}" "${ANDROID_ABI}"
#
#        ) #OUTPUT ${CMAKE_SOURCE_DIR}/../contrib/tap-protocol/contrib/bitcoin-core/src/secp256k1/build/android/${ANDROID_ABI}/lib/libsecp256k1.a
#else()
#        add_custom_command(
#                TARGET cktap_protocol_plugin PRE_BUILD
#                COMMAND "${CMAKE_SOURCE_DIR}/build_libsecp256k1.sh" ARGS "${CMAKE_ANDROID_NDK}" "${ANDROID_ABI}"
#
#        ) #OUTPUT ${CMAKE_SOURCE_DIR}/../contrib/tap-protocol/contrib/bitcoin-core/src/secp256k1/build/android/${ANDROID_ABI}/lib64/libsecp256k1.a
#endif()