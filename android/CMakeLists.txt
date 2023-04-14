cmake_minimum_required(VERSION 3.16)

project(cktap_protocol_plugin)

add_library(cktap_protocol_plugin

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        src/main/cpp/test.cpp)

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

# Ensure libsecp256k1 is built for the Nunchuk tap-protocol library to link
execute_process(COMMAND "${CMAKE_SOURCE_DIR}/build_libsecp256k1.sh" "${CMAKE_ANDROID_NDK}" "${ANDROID_ABI}")
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

add_subdirectory("${CMAKE_SOURCE_DIR}/../contrib/tap-protocol" "${CMAKE_SOURCE_DIR}/../build/tap-protocol")
target_link_libraries("${PROJECT_NAME}" PUBLIC tap-protocol)
