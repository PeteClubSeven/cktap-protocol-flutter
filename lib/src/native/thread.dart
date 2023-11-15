import 'dart:async';
import 'dart:ffi';
import 'dart:typed_data';

import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/exceptions.dart';
import 'package:cktap_protocol/src/error/types.dart';
import 'package:cktap_protocol/src/error/validation.dart';
import 'package:cktap_protocol/src/native/bindings.dart';
import 'package:cktap_protocol/src/native/factories.dart';
import 'package:cktap_protocol/src/native/library.dart';
import 'package:cktap_protocol/transport.dart';

/// Attempts to cancel the current operation and waits until it has
Future<void> cancelNativeOperation() async {
  return Future.sync(() async {
    // We don't need to do anything if the thread is inactive
    if (!_isNativeThreadActive()) {
      return;
    }

    ensure(nativeLibrary.Core_requestCancelOperation());
    final stopwatch = Stopwatch()..start();
    while (_isNativeThreadActive()) {
      if (stopwatch.elapsed.inSeconds >= 2) {
        throw TimeoutException(
            "CKTapProtocol couldn't cancel the native operation");
      }
      await Future.delayed(const Duration(microseconds: 50));
    }

    ensureNativeThreadState(CKTapThreadState.canceled);
    var errorCode = nativeLibrary.Core_finalizeAsyncAction();
    if (errorCode != CKTapInterfaceErrorCode.operationCanceled) {
      ensure(errorCode);
    }
  });
}

/// Tries to retrieve the card data from the native thread and return in a
/// Dart-native format
CKTapCard finalizeCardCreation() {
  int threadState = nativeLibrary.Core_getThreadState();
  if (threadState == CKTapThreadState.finished) {
    CKTapOperationResponse response = nativeLibrary.Core_endOperation();
    ensure(response.errorCode);

    switch (response.handle.type) {
      case CKTapCardType.satscard:
      case CKTapCardType.tapsigner:
        return makeCardFromHandle(response.handle);
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
void prepareForCardHandshake(CardType type) {
  ensureNativeThreadState(CKTapThreadState.notStarted);
  ensure(nativeLibrary.Core_beginAsyncHandshake(type.index));
  ensureNativeThreadState(CKTapThreadState.asyncActionStarting);
}

/// Prepares the native thread for performing a specific operation on an already
/// constructed card
void prepareForCardOperation(int handle, CardType type) {
  ensureNativeThreadState(CKTapThreadState.notStarted);
  ensure(nativeLibrary.Core_prepareCardOperation(handle, type.index));
}

/// Attempts to return the native thread to a workable clean state
void prepareNativeThread() {
  if (_isNativeThreadActive()) {
    throw ProtocolConcurrencyError("Can't prepare the native thread");
  }

  ensure(nativeLibrary.Core_newOperation());
}

/// Handles the sending and receiving of data between the native library and an
/// NFC device until completion or failure
Future<void> processTransportRequests(Transport transport) async {
  return Future.sync(() async {
    ensureNativeThreadStates([
      CKTapThreadState.asyncActionStarting,
      CKTapThreadState.awaitingTransportRequest,
      CKTapThreadState.transportRequestReady
    ]);

    while (_isNativeThreadActive()) {
      // Allow the background thread to reach the desired state
      if (nativeLibrary.Core_getThreadState() !=
          CKTapThreadState.transportRequestReady) {
        await Future.delayed(const Duration(microseconds: 50));
        continue;
      }

      // Communicate any necessary data with the NFC card
      // TODO: Adjust for iOS
      var transportResponse =
          await transport.sendBytes(_getNativeTransportRequest());
      var errorCode = _setNativeTransportResponse(transportResponse);
      ensure(errorCode);
    }

    if (nativeLibrary.Core_getThreadState() ==
        CKTapThreadState.invalidCardProduced) {
      throw InvalidCardException();
    }
    ensure(nativeLibrary.Core_finalizeAsyncAction());
  });
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
  ensure(CKTapInterfaceErrorCode.success);

  return errorCode;
}
