#ifndef __CKTAP_PROTOCOL__STRUCTS_H__
#define __CKTAP_PROTOCOL__STRUCTS_H__

// Project
#include <enums.h>
#include <macros.h>

// libc
#include <stdint.h>

FFI_PLUGIN_EXPORT typedef struct {
    uint8_t* ptr;
    int32_t length;
} CBinaryArray;

FFI_PLUGIN_EXPORT typedef struct {
    int32_t index;
    CKTapCardType type;
} CKTapCardHandle;

FFI_PLUGIN_EXPORT typedef struct {
    CKTapCardHandle handle;
    CKTapInterfaceErrorCode errorCode;
} CKTapOperationResponse;

FFI_PLUGIN_EXPORT typedef struct {
    int32_t code;
    char* message;
} CKTapProtoException;

FFI_PLUGIN_EXPORT typedef struct {
    int32_t index;
    int32_t status;
    char* address;

    // Requires the CVC to acquire
    CBinaryArray privkey;
    CBinaryArray pubkey;
    CBinaryArray masterPK;
    CBinaryArray chainCode;
} IntermediateSatscardSlot;

#endif // __CKTAP_PROTOCOL__STRUCTS_H__