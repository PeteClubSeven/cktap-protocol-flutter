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
    int32_t code;
    char* message;
} CKTapProtoException;

FFI_PLUGIN_EXPORT typedef struct {
    int32_t index;
    CKTapCardType type;
} CKTapCardHandle;

FFI_PLUGIN_EXPORT typedef struct {
    CKTapCardHandle handle;
    CKTapInterfaceErrorCode errorCode;
} CKTapOperationResponse;

/// Used when accessing tap_protocol methods that can throw
FFI_PLUGIN_EXPORT typedef struct {
    CKTapInterfaceErrorCode errorCode;
    CKTapProtoException exception;
} CKTapInterfaceStatus;

FFI_PLUGIN_EXPORT typedef struct {
    int32_t handle;
    int32_t type;
    char* ident;
    char* appletVersion;
    int32_t authDelay;
    int32_t birthHeight;
    int8_t isCertsChecked;
    int8_t isTampered;
    int8_t isTestnet;
    int8_t needsSetup;
} CKTapCardConstructorParams;

FFI_PLUGIN_EXPORT typedef struct {
    int32_t satscardHandle;
    int32_t index;
    int32_t status;
    char* address;

    /// Requires the CVC to acquire
    CBinaryArray privkey;
    CBinaryArray pubkey;
    CBinaryArray masterPK;
    CBinaryArray chainCode;
} SlotConstructorParams;

FFI_PLUGIN_EXPORT typedef struct {
    CKTapInterfaceStatus status;
    char* wif;
} SlotToWifResponse;

FFI_PLUGIN_EXPORT typedef struct {
    CKTapInterfaceStatus status;
    CKTapCardConstructorParams base;
    int32_t activeSlotIndex;
    int32_t numSlots;
    int8_t hasUnusedSlots;
    int8_t isUsedUp;
} SatscardConstructorParams;

FFI_PLUGIN_EXPORT typedef struct {
    CKTapInterfaceStatus status;
    SlotConstructorParams params;
} SatscardGetSlotResponse;

FFI_PLUGIN_EXPORT typedef struct {
    CKTapInterfaceStatus status;
    CKTapCardConstructorParams base;
    int32_t numberOfBackups;
    char* derivationPath;
} TapsignerConstructorParams;

#endif // __CKTAP_PROTOCOL__STRUCTS_H__