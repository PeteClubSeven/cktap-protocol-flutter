import 'dart:async';
import 'dart:ffi';
import 'dart:typed_data';

import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/exceptions.dart';
import 'package:cktap_protocol/src/error/types.dart';
import 'package:cktap_protocol/src/error/validation.dart';
import 'package:cktap_protocol/src/native/bindings.dart';
import 'package:cktap_protocol/src/native/library.dart';
import 'package:cktap_protocol/src/nfc_bridge.dart';

/// Attempts to return the native thread to a workable clean state
Future<void> prepareNativeThread() async {
  if (_isNativeThreadActive()) {
    throw ProtocolConcurrencyError("Can't prepare the native thread");
  }

  ensureSuccessful(nativeLibrary.Core_InitializeLibrary());
  ensureSuccessful(nativeLibrary.Core_NewOperation());
}

/// Tries to retrieve the card data from the native thread and return in a Dart-native format
Future<CKTapCard> finalizeCardCreation() async {
  int threadState = nativeLibrary.Core_GetThreadState();
  if (threadState == CKTapThreadState.Finished) {
    CKTapOperationResponse response = nativeLibrary.Core_EndOperation();
    ensureSuccessful(response.errorCode);

    switch (response.handle.type) {
      case CKTapCardType.Satscard:
        return Satscard(response.handle.index, response.handle.type);
      case CKTapCardType.Tapsigner:
        return Tapsigner(response.handle.index, response.handle.type);
      default:
        throw UnsupportedError(
            "Can't create CKTapCard of type: ${response.handle.type}");
    }
  }

  switch (threadState) {
    case CKTapThreadState.Timeout:
      throw TimeoutException("CKTap card creation timed out");
    case CKTapThreadState.Canceled:
      throw OperationCanceledException("CKTap card creation canceled");
    default:
      throw InvalidThreadStateError(CKTapThreadState.Finished, threadState);
  }
}

/// Tells the native thread to start the handshaking process
Future<void> prepareForCardHandshake() async {
  ensureNativeThreadState(CKTapThreadState.NotStarted);
  ensureSuccessful(nativeLibrary.Core_BeginAsyncHandshake());
}

/// Handles the sending and receiving of data between the native library and an
/// NFC device until completion or failure
Future<void> processTransportRequests(NfcBridge nfc) async {
  ensureNativeThreadStates([
    CKTapThreadState.AwaitingTransportRequest,
    CKTapThreadState.TransportRequestReady
  ]);

  while (_isNativeThreadActive()) {
    // Allow the background thread to reach the desired state
    if (nativeLibrary.Core_GetThreadState() !=
        CKTapThreadState.TransportRequestReady) {
      await Future.delayed(Duration.zero);
      continue;
    }

    // Communicate any necessary data with the NFC card
    var transportResponse = await nfc.send(_getNativeTransportRequest());
    var errorCode = _setNativeTransportResponse(transportResponse);
    ensureSuccessful(errorCode);
  }

  ensureSuccessful(nativeLibrary.Core_FinalizeAsyncAction());
}

/// Stops transport request loops when given any "final" states"
bool _isNativeThreadActive() {
  int threadState = nativeLibrary.Core_GetThreadState();
  return threadState != CKTapThreadState.NotStarted &&
      threadState < CKTapThreadState.Finished;
}

/// Converts the native transport request to a Dart readable format.
/// Should only be called when there is a transport request ready.
Uint8List _getNativeTransportRequest() {
  assert(nativeLibrary.Core_GetThreadState() ==
      CKTapThreadState.TransportRequestReady);

  Pointer<Uint8> requestPointer =
      nativeLibrary.Core_GetTransportRequestPointer();
  int requestLength = nativeLibrary.Core_GetTransportRequestLength();
  assert(requestPointer.address != 0);
  assert(requestLength != 0);

  return requestPointer.asTypedList(requestLength);
}

/// Passes a Dart-typed response from an NFC device to the native thread.
/// Should only be called when there is a transport request ready.
int _setNativeTransportResponse(Uint8List response) {
  assert(response.isNotEmpty);
  assert(nativeLibrary.Core_GetThreadState() ==
      CKTapThreadState.TransportRequestReady);

  Pointer<Uint8> allocation =
      nativeLibrary.Core_AllocateTransportResponseBuffer(response.length);
  assert(allocation.address != 0);

  var nativeResponse = allocation.asTypedList(response.length);
  nativeResponse.setAll(0, response);

  var errorCode = nativeLibrary.Core_FinalizeTransportResponse();
  ensureSuccessful(CKTapInterfaceErrorCode.Success);

  return errorCode;
}
