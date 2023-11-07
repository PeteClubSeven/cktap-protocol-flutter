#ifndef __CKTAP_PROTOCOL__EXPORTS_H__
#define __CKTAP_PROTOCOL__EXPORTS_H__

// Project
#include "Structs.h"

// libc
#include "stdint.h"

// ----------------------------------------------
// Core Bindings:

/// Ensures the library and native thread are initialized
FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_InitializeLibrary();

/// Must be called first to restore the native thread to its initial state
FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_NewOperation();
/// Must be called last to store and retrieve Satscard/Tapsigner data
FFI_PLUGIN_EXPORT CKTapOperationResponse Core_EndOperation();

/// Attempts to perform an initial handshake with a CKTapCard
FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_BeginAsyncHandshake();
/// Must be called at the end of every async action
FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_FinalizeAsyncAction();

/// Retrieves a pointer to the current transport request
/// Returns nullptr if the native thread isn't ready or is invalid
FFI_PLUGIN_EXPORT const uint8_t* Core_GetTransportRequestPointer();
/// Retrieves the size of the current transport request in bytes
/// Returns 0 if the native thread isn't ready or is invalid
FFI_PLUGIN_EXPORT int32_t Core_GetTransportRequestLength();

/// Ensures that the transport response buffer will be appropriately sized
/// Returns a pointer to the buffer if valid, nullptr if not
FFI_PLUGIN_EXPORT uint8_t* Core_AllocateTransportResponseBuffer(const int32_t sizeInBytes);
/// Informs the native thread that it's now safe to read the previously allocated buffer
FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_FinalizeTransportResponse();

/// Gets the current native thread state atomically
FFI_PLUGIN_EXPORT CKTapThreadState Core_GetThreadState();
/// Gets the most recent tap_protocol::TapProtoException ONLY if the current thread state is
/// CKTapThreadState::TapProtocolError
FFI_PLUGIN_EXPORT CKTapProtoException Core_GetTapProtoException();

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
