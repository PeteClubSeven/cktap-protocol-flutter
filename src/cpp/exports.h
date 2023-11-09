#ifndef __CKTAP_PROTOCOL__EXPORTS_H__
#define __CKTAP_PROTOCOL__EXPORTS_H__

// Project
#include <structs.h>

// libc
#include <stdint.h>

// ----------------------------------------------
// Core Bindings:

/// Ensures the library and native thread are initialized
FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_initializeLibrary();

/// Must be called first to restore the native thread to its initial state
FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_newOperation();
/// Must be called last to store and retrieve Satscard/Tapsigner data
FFI_PLUGIN_EXPORT CKTapOperationResponse Core_endOperation();
/// Signals cancellation of the current operation, causing the thread to enter a
/// resettable state
FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_requestCancelOperation();

/// Attempts to perform an initial handshake with a CKTapCard
FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_beginAsyncHandshake();
/// Must be called at the end of every async action
FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_finalizeAsyncAction();

/// Retrieves a pointer to the current transport request
/// Returns nullptr if the native thread isn't ready or is invalid
FFI_PLUGIN_EXPORT const uint8_t* Core_getTransportRequestPointer();
/// Retrieves the size of the current transport request in bytes
/// Returns 0 if the native thread isn't ready or is invalid
FFI_PLUGIN_EXPORT int32_t Core_getTransportRequestLength();

/// Ensures that the transport response buffer will be appropriately sized
/// Returns a pointer to the buffer if valid, nullptr if not
FFI_PLUGIN_EXPORT uint8_t* Core_allocateTransportResponseBuffer(int32_t sizeInBytes);
/// Informs the native thread that it's now safe to read the previously allocated buffer
FFI_PLUGIN_EXPORT CKTapInterfaceErrorCode Core_finalizeTransportResponse();

/// Gets the current native thread state atomically
FFI_PLUGIN_EXPORT CKTapThreadState Core_getThreadState();
/// Gets the most recent tap_protocol::TapProtoException ONLY if the current thread state is
/// CKTapThreadState::tapProtocolError
FFI_PLUGIN_EXPORT CKTapProtoException Core_getTapProtoException();

// ----------------------------------------------
// CKTapCard:
// TODO: Simply data retrieval to avoid overhead of so many FFI calls and lookups
FFI_PLUGIN_EXPORT char* CKTapCard_getIdentCString(int32_t handle, int32_t type);
FFI_PLUGIN_EXPORT char* CKTapCard_getAppletVersionCString(int32_t handle, int32_t type);
FFI_PLUGIN_EXPORT int32_t CKTapCard_getBirthHeight(int32_t handle, int32_t type);
FFI_PLUGIN_EXPORT int32_t CKTapCard_isTestnet(int32_t handle, int32_t type);
FFI_PLUGIN_EXPORT int32_t CKTapCard_getAuthDelay(int32_t handle, int32_t type);
FFI_PLUGIN_EXPORT int32_t CKTapCard_isTampered(int32_t handle, int32_t type);
FFI_PLUGIN_EXPORT int32_t CKTapCard_isCertsChecked(int32_t handle, int32_t type);
FFI_PLUGIN_EXPORT int32_t CKTapCard_needSetup(int32_t handle, int32_t type);

// ----------------------------------------------
// Satscard:
// TODO: Simply data retrieval to avoid overhead of so many FFI calls and lookups
FFI_PLUGIN_EXPORT IntermediateSatscardSlot Satscard_getActiveSlot(int32_t handle, int32_t type);
FFI_PLUGIN_EXPORT int32_t Satscard_getActiveSlotIndex(int32_t handle, int32_t type);
FFI_PLUGIN_EXPORT int32_t Satscard_getNumSlots(int32_t handle, int32_t type);
FFI_PLUGIN_EXPORT int32_t Satscard_hasUnusedSlots(int32_t handle, int32_t type);
FFI_PLUGIN_EXPORT int32_t Satscard_isUsedUp(int32_t handle, int32_t type);

// ----------------------------------------------
// Tapsigner:
// TODO: Simply data retrieval to avoid overhead of so many FFI calls and lookups
FFI_PLUGIN_EXPORT int32_t Tapsigner_getNumberOfBackups(int32_t handle, int32_t type);
FFI_PLUGIN_EXPORT char* Tapsigner_getDerivationPath(int32_t handle, int32_t type);

// ----------------------------------------------
// Utility:
FFI_PLUGIN_EXPORT void Utility_freeBinaryArray(CBinaryArray array);
FFI_PLUGIN_EXPORT void Utility_freeIntermediateSatscardSlot(IntermediateSatscardSlot slot);
FFI_PLUGIN_EXPORT void Utility_freeString(char* cString);

#endif // __CKTAP_PROTOCOL__EXPORTS_H__
