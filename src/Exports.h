#ifndef __CKTAP_PROTOCOL__EXPORTS_H__
#define __CKTAP_PROTOCOL__EXPORTS_H__

// Project
#include "Enums.h"
#include "Macros.h"

// libc
#include "stdint.h"

FFI_PLUGIN_EXPORT CKTapThreadState CKTapCard_GetThreadState();

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode CKTapCard_BeginInitialization();

FFI_PLUGIN_EXPORT const uint8_t* CKTapCard_GetTransportRequestPointer();
FFI_PLUGIN_EXPORT int32_t CKTapCard_GetTransportRequestLength();

FFI_PLUGIN_EXPORT uint8_t* CKTapCard_AllocateTransportResponseBuffer(const int32_t sizeInBytes);
FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode CKTapCard_FinalizeTransportResponse();

#endif // __CKTAP_PROTOCOL__EXPORTS_H__
