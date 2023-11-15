#ifndef __CKTAP_PROTOCOL__EXPORTS_H__
#define __CKTAP_PROTOCOL__EXPORTS_H__

// Project
#include <structs.h>

// libc
#include <stdint.h>

// ----------------------------------------------
// Core Bindings:

/// Ensures the library and native thread are initialized
FFI_FUNC_EXPORT CKTapInterfaceErrorCode Core_initializeLibrary();

/// Must be called first to restore the native thread to its initial state
FFI_FUNC_EXPORT CKTapInterfaceErrorCode Core_newOperation();
/// Must be called last to store and retrieve Satscard/Tapsigner data
FFI_FUNC_EXPORT CKTapOperationResponse Core_endOperation();
/// Signals cancellation of the current operation, causing the thread to enter a
/// resettable state
FFI_FUNC_EXPORT CKTapInterfaceErrorCode Core_requestCancelOperation();

/// Searches for the specified card and gives the native thread access so
/// further operations can be performed on it
FFI_FUNC_EXPORT CKTapInterfaceErrorCode Core_prepareCardOperation(int32_t handle, int32_t cardType);
/// Attempts to perform an initial handshake with a CKTapCard
FFI_FUNC_EXPORT CKTapInterfaceErrorCode Core_beginAsyncHandshake(int32_t cardType);
/// Must be called at the end of every async action
FFI_FUNC_EXPORT CKTapInterfaceErrorCode Core_finalizeAsyncAction();

/// Retrieves a pointer to the current transport request
/// Returns nullptr if the native thread isn't ready or is invalid
FFI_FUNC_EXPORT const uint8_t* Core_getTransportRequestPointer();
/// Retrieves the size of the current transport request in bytes
/// Returns 0 if the native thread isn't ready or is invalid
FFI_FUNC_EXPORT int32_t Core_getTransportRequestLength();

/// Ensures that the transport response buffer will be appropriately sized
/// Returns a pointer to the buffer if valid, nullptr if not
FFI_FUNC_EXPORT uint8_t* Core_allocateTransportResponseBuffer(int32_t sizeInBytes);
/// Informs the native thread that it's now safe to read the previously allocated buffer
FFI_FUNC_EXPORT CKTapInterfaceErrorCode Core_finalizeTransportResponse();

/// Gets the current native thread state atomically
FFI_FUNC_EXPORT CKTapThreadState Core_getThreadState();
/// Gets the most recent tap_protocol::TapProtoException ONLY if the current thread state is
/// CKTapThreadState::tapProtocolError
FFI_FUNC_EXPORT CKTapProtoException Core_getTapProtoException();

// ----------------------------------------------
// CKTapCard:

FFI_FUNC_EXPORT CKTapInterfaceErrorCode CKTapCard_beginWait();
FFI_FUNC_EXPORT WaitResponseParams CKTapCard_getWaitResponse();

// ----------------------------------------------
// Satscard:

/// Gets a C representation of parameters required to construct a [Satscard] in dart. Note: must use
/// [Utility_freeSatscardConstructorParams] when you are finished using the data to deallocate memory
FFI_FUNC_EXPORT SatscardConstructorParams Satscard_createConstructorParams(int32_t handle);
FFI_FUNC_EXPORT SatscardSlotResponse Satscard_getActiveSlot(int32_t handle);
FFI_FUNC_EXPORT SlotToWifResponse Satscard_slotToWif(int32_t handle, int32_t index);

FFI_FUNC_EXPORT CKTapInterfaceErrorCode Satscard_beginCertificateCheck();
FFI_FUNC_EXPORT CKTapInterfaceErrorCode Satscard_beginGetSlot(int32_t slot, const char* spendCode);
FFI_FUNC_EXPORT CKTapInterfaceErrorCode Satscard_beginListSlots(const char* spendCode, int32_t limit);
FFI_FUNC_EXPORT CKTapInterfaceErrorCode Satscard_beginNew(const char* chainCode, const char* spendCode);
FFI_FUNC_EXPORT CKTapInterfaceErrorCode Satscard_beginUnseal(const char* spendCode);

FFI_FUNC_EXPORT CertificateCheckParams Satscard_getCertificateCheckResponse();
FFI_FUNC_EXPORT SatscardSlotResponse Satscard_getGetSlotResponse(int32_t handle);
FFI_FUNC_EXPORT SatscardListSlotsParams Satscard_getListSlotsResponse(int32_t handle);
FFI_FUNC_EXPORT SatscardSlotResponse Satscard_getNewResponse(int32_t handle);
FFI_FUNC_EXPORT SatscardSlotResponse Satscard_getUnsealResponse(int32_t handle);

// ----------------------------------------------
// Tapsigner:

/// Gets a C representation of parameters required to construct a [Tapsigner] in dart. Note: must use
/// [Utility_freeTapsignerConstructorParams] when you are finished using the data to deallocate memory
FFI_FUNC_EXPORT TapsignerConstructorParams Tapsigner_createConstructorParams(int32_t handle);

// ----------------------------------------------
// Utility:

FFI_FUNC_EXPORT void Utility_freeCBinaryArray(CBinaryArray array);
FFI_FUNC_EXPORT void Utility_freeCKTapInterfaceStatus(CKTapInterfaceStatus status);
FFI_FUNC_EXPORT void Utility_freeCKTapProtoException(CKTapProtoException exception);
FFI_FUNC_EXPORT void Utility_freeCString(char* cString);
FFI_FUNC_EXPORT void Utility_freeSatscardSlotResponse(SatscardSlotResponse response);
FFI_FUNC_EXPORT void Utility_freeSatscardConstructorParams(SatscardConstructorParams params);
FFI_FUNC_EXPORT void Utility_freeSatscardListSlotsParams(SatscardListSlotsParams params);
FFI_FUNC_EXPORT void Utility_freeSlotConstructorParams(SlotConstructorParams params);
FFI_FUNC_EXPORT void Utility_freeSlotToWifResponse(SlotToWifResponse response);
FFI_FUNC_EXPORT void Utility_freeTapsignerConstructorParams(TapsignerConstructorParams params);

#endif // __CKTAP_PROTOCOL__EXPORTS_H__
