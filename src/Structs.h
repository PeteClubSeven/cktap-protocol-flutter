#ifndef __CKTAP_PROTOCOL__STRUCTS_H__
#define __CKTAP_PROTOCOL__STRUCTS_H__

// Project
#include "Enums.h"
#include "Macros.h"

// libc
#include "stdint.h"

FFI_PLUGIN_EXPORT typedef struct 
{
    int32_t index;
    CKTapCardType type;
} CKTapCardHandle;

FFI_PLUGIN_EXPORT typedef struct 
{
    CKTapCardHandle handle;
    CKTapInterfaceErrorCode errorCode;
} CKTapCardFinalizeOperationResponse;

#endif // __CKTAP_PROTOCOL__STRUCTS_H__