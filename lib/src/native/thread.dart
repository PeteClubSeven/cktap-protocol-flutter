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

/// Tries to retrieve the card data from the native thread and return in a Dart-native format
Future<CKTapCard> finalizeCardCreation() async {
  int threadState = nativeLibrary.Core_getThreadState();
  if (threadState == CKTapThreadState.finished) {
    CKTapOperationResponse response = nativeLibrary.Core_endOperation();
    ensureSuccessful(response.errorCode);

    switch (response.handle.type) {
      case CKTapCardType.satscard:
        return Satscard(response.handle.index, response.handle.type);
      case CKTapCardType.tapsigner:
        return Tapsigner(response.handle.index, response.handle.type);
      default:
        throw UnsupportedError(
            "Can't create CKTapCard of type: ${response.handle.type}");
    }
  }

  switch (threadState) {
    case CKTapThreadState.timeout:
      throw TimeoutException("CKTap card creation timed out");
    case CKTapThreadState.canceled:
      throw OperationCanceledException("CKTap card creation canceled");
    default:
      throw InvalidThreadStateError(CKTapThreadState.finished, threadState);
  }
}

/// Tells the native thread to start the handshaking process
Future<void> prepareForCardHandshake() async {
  ensureNativeThreadState(CKTapThreadState.notStarted);
  ensureSuccessful(nativeLibrary.Core_beginAsyncHandshake());
}

/// Attempts to return the native thread to a workable clean state
Future<void> prepareNativeThread() async {
  if (_isNativeThreadActive()) {
    throw ProtocolConcurrencyError("Can't prepare the native thread");
  }

  ensureSuccessful(nativeLibrary.Core_newOperation());
}

/// Handles the sending and receiving of data between the native library and an
/// NFC device until completion or failure
Future<void> processTransportRequests(NfcBridge nfc) async {
  ensureNativeThreadStates([
    CKTapThreadState.asyncActionStarting,
    CKTapThreadState.awaitingTransportRequest,
    CKTapThreadState.transportRequestReady
  ]);

  while (_isNativeThreadActive()) {
    // Allow the background thread to reach the desired state
    if (nativeLibrary.Core_getThreadState() !=
        CKTapThreadState.transportRequestReady) {
      await Future.delayed(Duration.zero);
      continue;
    }

    // Communicate any necessary data with the NFC card
    var transportResponse = await nfc.send(_getNativeTransportRequest());
    var errorCode = _setNativeTransportResponse(transportResponse);
    ensureSuccessful(errorCode);
  }

  ensureSuccessful(nativeLibrary.Core_finalizeAsyncAction());
}

/// Converts the native transport request to a Dart readable format.
/// Should only be called when there is a transport request ready.
Uint8List _getNativeTransportRequest() {
  assert(nativeLibrary.Core_getThreadState() ==
      CKTapThreadState.transportRequestReady);

  Pointer<Uint8> requestPointer =
      nativeLibrary.Core_getTransportRequestPointer();
  int requestLength = nativeLibrary.Core_getTransportRequestLength();
  assert(requestPointer.address != 0);
  assert(requestLength != 0);

  return requestPointer.asTypedList(requestLength);
}

/// Stops transport request loops when given any "final" states"
bool _isNativeThreadActive() {
  int threadState = nativeLibrary.Core_getThreadState();
  return threadState != CKTapThreadState.notStarted &&
      threadState < CKTapThreadState.finished;
}

/// Passes a Dart-typed response from an NFC device to the native thread.
/// Should only be called when there is a transport request ready.
int _setNativeTransportResponse(Uint8List response) {
  assert(response.isNotEmpty);
  assert(nativeLibrary.Core_getThreadState() ==
      CKTapThreadState.transportRequestReady);

  Pointer<Uint8> allocation =
      nativeLibrary.Core_allocateTransportResponseBuffer(response.length);
  assert(allocation.address != 0);

  var nativeResponse = allocation.asTypedList(response.length);
  nativeResponse.setAll(0, response);

  var errorCode = nativeLibrary.Core_finalizeTransportResponse();
  ensureSuccessful(CKTapInterfaceErrorCode.success);

  return errorCode;
}
