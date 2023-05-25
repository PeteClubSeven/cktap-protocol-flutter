#ifndef __CKTAP_PROTOCOL__EXPORTS_H__
#define __CKTAP_PROTOCOL__EXPORTS_H__

// Project
#include "Structs.h"

// libc
#include "stdint.h"

// ----------------------------------------------
// Core Bindings: 
FFI_PLUGIN_EXPORT CKTapThreadState Core_GetThreadState();

FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_BeginInitialization();
FFI_PLUGIN_EXPORT CKTapCardFinalizeOperationResponse Core_FinalizeRecentOperation();

FFI_PLUGIN_EXPORT const uint8_t* Core_GetTransportRequestPointer();
FFI_PLUGIN_EXPORT int32_t Core_GetTransportRequestLength();

FFI_PLUGIN_EXPORT uint8_t* Core_AllocateTransportResponseBuffer(const int32_t sizeInBytes);
FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_FinalizeTransportResponse();

// ----------------------------------------------
// CKTapCard:
FFI_PLUGIN_EXPORT char* CKTapCard_GetIdentCString(const int32_t handle, const int32_t type);
FFI_PLUGIN_EXPORT char* CKTapCard_GetAppletVersionCString(const int32_t handle, const int32_t type);
FFI_PLUGIN_EXPORT int32_t CKTapCard_GetBirthHeight(const int32_t handle, const int32_t type);
FFI_PLUGIN_EXPORT int32_t CKTapCard_IsTestnet(const int32_t handle, const int32_t type);
FFI_PLUGIN_EXPORT int32_t CKTapCard_GetAuthDelay(const int32_t handle, const int32_t type);
FFI_PLUGIN_EXPORT int32_t CKTapCard_IsTampered(const int32_t handle, const int32_t type);
FFI_PLUGIN_EXPORT int32_t CKTapCard_IsCertsChecked(const int32_t handle, const int32_t type);
FFI_PLUGIN_EXPORT int32_t CKTapCard_NeedSetup(const int32_t handle, const int32_t type);

// ----------------------------------------------
// Satscard:
FFI_PLUGIN_EXPORT IntermediateSatscardSlot Satscard_GetActiveSlot(const int32_t handle, const int32_t type);
FFI_PLUGIN_EXPORT int32_t Satscard_GetActiveSlotIndex(const int32_t handle, const int32_t type);
FFI_PLUGIN_EXPORT int32_t Satscard_GetNumSlots(const int32_t handle, const int32_t type);
FFI_PLUGIN_EXPORT int32_t Satscard_HasUnusedSlots(const int32_t handle, const int32_t type);
FFI_PLUGIN_EXPORT int32_t Satscard_IsUsedUp(const int32_t handle, const int32_t type);

// ----------------------------------------------
// Tapsigner:
FFI_PLUGIN_EXPORT int32_t Tapsigner_GetNumberOfBackups(const int32_t handle, const int32_t type);
FFI_PLUGIN_EXPORT char* Tapsigner_GetDerivationPath(const int32_t handle, const int32_t type);

// ----------------------------------------------
// Utility:
FFI_PLUGIN_EXPORT void Utility_FreeBinaryArray(CBinaryArray array);
FFI_PLUGIN_EXPORT void Utility_FreeIntermediateSatscardSlot(IntermediateSatscardSlot slot);
FFI_PLUGIN_EXPORT void Utility_FreeString(char* cString);

#endif // __CKTAP_PROTOCOL__EXPORTS_H__
