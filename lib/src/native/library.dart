import 'dart:ffi';
import 'dart:io';

import 'package:cktap_protocol/src/implementation.dart';
import 'package:cktap_protocol/src/native/bindings.dart';

/// Gets an instance of the native library bindings
NativeBindings get nativeLibrary => Implementation.instance.bindings;

/// Loads the library of the given name in the expected format for the current platform
DynamicLibrary loadLibrary(final String libName) {
  if (Platform.isMacOS || Platform.isIOS) {
    return DynamicLibrary.open('$libName.framework/$libName');
  }
  if (Platform.isAndroid || Platform.isLinux) {
    return DynamicLibrary.open('lib$libName.so');
  }
  if (Platform.isWindows) {
    return DynamicLibrary.open('$libName.dll');
  }
  throw UnsupportedError(
      'Unknown platform (${Platform.operatingSystem}) for lib: $libName');
}

/// Maps internal interface error code numbers to a string-readable version
final Map<int, String> tapInterfaceErrorLiteralMap = {
  CKTapInterfaceErrorCode.pending: "pending",
  CKTapInterfaceErrorCode.success: "success",
  CKTapInterfaceErrorCode.attemptToFinalizeActiveThread:
      "attemptToFinalizeActiveThread",
  CKTapInterfaceErrorCode.bindingNotImplemented: "bindingNotImplemented",
  CKTapInterfaceErrorCode.caughtTapProtocolException:
      "caughtTapProtocolException",
  CKTapInterfaceErrorCode.expectedSatscardButReceivedNothing:
      "expectedSatscardButReceivedNothing",
  CKTapInterfaceErrorCode.expectedTapsignerButReceivedNothing:
      "expectedTapsignerButReceivedNothing",
  CKTapInterfaceErrorCode.failedToPerformHandshake: "failedToPerformHandshake",
  CKTapInterfaceErrorCode.failedToRetrieveValueFromFuture:
      "failedToRetrieveValueFromFuture",
  CKTapInterfaceErrorCode.invalidCardDuringHandshake:
      "invalidCardDuringHandshake",
  CKTapInterfaceErrorCode.invalidCardOperation: "invalidCardOperation",
  CKTapInterfaceErrorCode.invalidHandlingOfCardDuringFinalization:
      "invalidHandlingOfCardDuringFinalization",
  CKTapInterfaceErrorCode.invalidResponseFromCardOperation:
      "invalidResponseFromCardOperation",
  CKTapInterfaceErrorCode.invalidThreadStateDuringTransportSignaling:
      "invalidThreadStateDuringTransportSignaling",
  CKTapInterfaceErrorCode.libraryNotInitialized: "libraryNotInitialized",
  CKTapInterfaceErrorCode.operationCanceled: "operationCanceled",
  CKTapInterfaceErrorCode.operationFailed: "operationFailed",
  CKTapInterfaceErrorCode.operationStillInProgress: "operationStillInProgress",
  CKTapInterfaceErrorCode.threadAlreadyInUse: "threadAlreadyInUse",
  CKTapInterfaceErrorCode.threadAllocationFailed: "threadAllocationFailed",
  CKTapInterfaceErrorCode.threadNotReadyForResponse:
      "threadNotReadyForResponse",
  CKTapInterfaceErrorCode.threadNotResetForHandshake:
      "threadNotResetForHandshake",
  CKTapInterfaceErrorCode.threadNotAwaitingCardOperation:
      "threadNotAwaitingCardOperation",
  CKTapInterfaceErrorCode.threadNotYetFinalized: "threadNotYetFinalized",
  CKTapInterfaceErrorCode.threadNotYetStarted: "threadNotYetStarted",
  CKTapInterfaceErrorCode.threadResponseFinalizationFailed:
      "threadResponseFinalizationFailed",
  CKTapInterfaceErrorCode.timeoutDuringTransport: "timeoutDuringTransport",
  CKTapInterfaceErrorCode.unableToFinalizeAsyncAction:
      "unableToFinalizeAsyncAction",
  CKTapInterfaceErrorCode.unexpectedExceptionWhenGettingCardOperationResult:
      "unexpectedExceptionWhenGettingCardOperationResult",
  CKTapInterfaceErrorCode.unexpectedExceptionWhenStartingCardOperation:
      "unexpectedExceptionWhenStartingCardOperation",
  CKTapInterfaceErrorCode.unexpectedStdException: "unexpectedStdException",
  CKTapInterfaceErrorCode.unknownErrorDuringAsyncOperation:
      "unknownErrorDuringAsyncOperation",
  CKTapInterfaceErrorCode.unknownErrorDuringHandshake:
      "unknownErrorDuringHandshake",
  CKTapInterfaceErrorCode.unknownErrorDuringTapProtocolFunction:
      "unknownErrorDuringTapProtocolFunction",
  CKTapInterfaceErrorCode.unknownSatscardHandle: "unknownSatscardHandle",
  CKTapInterfaceErrorCode.unknownSlotForGivenSatscardHandle:
      "unknownSlotForGivenSatscardHandle",
  CKTapInterfaceErrorCode.unknownTapsignerHandle: "unknownTapsignerHandle",
};

/// Maps Nunchuk tap_protocol error code numbers to a string-readable version
final Map<int, String> tapProtoExceptionErrorLiteralMap = {
  CKTapProtoExceptionErrorCode.INVALID_DEVICE: "INVALID_DEVICE",
  CKTapProtoExceptionErrorCode.UNLUCKY_NUMBER: "UNLUCKY_NUMBER",
  CKTapProtoExceptionErrorCode.BAD_ARGUMENTS: "BAD_ARGUMENTS",
  CKTapProtoExceptionErrorCode.BAD_AUTH: "BAD_AUTH",
  CKTapProtoExceptionErrorCode.NEED_AUTH: "NEED_AUTH",
  CKTapProtoExceptionErrorCode.UNKNOW_COMMAND: "UNKNOW_COMMAND",
  CKTapProtoExceptionErrorCode.INVALID_COMMAND: "INVALID_COMMAND",
  CKTapProtoExceptionErrorCode.INVALID_STATE: "INVALID_STATE",
  CKTapProtoExceptionErrorCode.WEAK_NONCE: "WEAK_NONCE",
  CKTapProtoExceptionErrorCode.BAD_CBOR: "BAD_CBOR",
  CKTapProtoExceptionErrorCode.BACKUP_FIRST: "BACKUP_FIRST",
  CKTapProtoExceptionErrorCode.RATE_LIMIT: "RATE_LIMIT",
  CKTapProtoExceptionErrorCode.DEFAULT_ERROR: "DEFAULT_ERROR",
  CKTapProtoExceptionErrorCode.MESSAGE_TOO_LONG: "MESSAGE_TOO_LONG",
  CKTapProtoExceptionErrorCode.MISSING_KEY: "MISSING_KEY",
  CKTapProtoExceptionErrorCode.ISO_SELECT_FAIL: "ISO_SELECT_FAIL",
  CKTapProtoExceptionErrorCode.SW_FAIL: "SW_FAIL",
  CKTapProtoExceptionErrorCode.INVALID_CVC_LENGTH: "INVALID_CVC_LENGTH",
  CKTapProtoExceptionErrorCode.PICK_KEY_PAIR_FAIL: "PICK_KEY_PAIR_FAIL",
  CKTapProtoExceptionErrorCode.ECDH_FAIL: "ECDH_FAIL",
  CKTapProtoExceptionErrorCode.XCVC_FAIL: "XCVC_FAIL",
  CKTapProtoExceptionErrorCode.UNKNOW_PROTO_VERSION: "UNKNOW_PROTO_VERSION",
  CKTapProtoExceptionErrorCode.INVALID_PUBKEY_LENGTH: "INVALID_PUBKEY_LENGTH",
  CKTapProtoExceptionErrorCode.NO_PRIVATE_KEY_PICKED: "NO_PRIVATE_KEY_PICKED",
  CKTapProtoExceptionErrorCode.MALFORMED_BIP32_PATH: "MALFORMED_BIP32_PATH",
  CKTapProtoExceptionErrorCode.INVALID_HASH_LENGTH: "INVALID_HASH_LENGTH",
  CKTapProtoExceptionErrorCode.SIG_VERIFY_ERROR: "SIG_VERIFY_ERROR",
  CKTapProtoExceptionErrorCode.INVALID_DIGEST_LENGTH: "INVALID_DIGEST_LENGTH",
  CKTapProtoExceptionErrorCode.INVALID_PATH_LENGTH: "INVALID_PATH_LENGTH",
  CKTapProtoExceptionErrorCode.SERIALIZE_ERROR: "SERIALIZE_ERROR",
  CKTapProtoExceptionErrorCode.EXCEEDED_RETRY: "EXCEEDED_RETRY",
  CKTapProtoExceptionErrorCode.INVALID_CARD: "INVALID_CARD",
  CKTapProtoExceptionErrorCode.SIGN_ERROR: "SIGN_ERROR",
  CKTapProtoExceptionErrorCode.SIG_TO_PUBKEY_FAIL: "SIG_TO_PUBKEY_FAIL",
  CKTapProtoExceptionErrorCode.PSBT_PARSE_ERROR: "PSBT_PARSE_ERROR",
  CKTapProtoExceptionErrorCode.PSBT_INVALID: "PSBT_INVALID",
  CKTapProtoExceptionErrorCode.INVALID_ADDRESS_TYPE: "INVALID_ADDRESS_TYPE",
  CKTapProtoExceptionErrorCode.INVALID_BACKUP_KEY: "INVALID_BACKUP_KEY",
  CKTapProtoExceptionErrorCode.INVALID_PUBKEY: "INVALID_PUBKEY",
  CKTapProtoExceptionErrorCode.INVALID_PRIVKEY: "INVALID_PRIVKEY",
  CKTapProtoExceptionErrorCode.INVALID_SLOT: "INVALID_SLOT",
};

/// Maps thread states to a string-readable version
final Map<int, String> tapThreadStateLiteralMap = {
  CKTapThreadState.notStarted: "notStarted",
  CKTapThreadState.awaitingCardOperation: "awaitingCardOperation",
  CKTapThreadState.asyncActionStarting: "asyncActionStarting",
  CKTapThreadState.awaitingTransportRequest: "awaitingTransportRequest",
  CKTapThreadState.transportRequestReady: "transportRequestReady",
  CKTapThreadState.transportResponseReady: "transportResponseReady",
  CKTapThreadState.processingTransportResponse: "processingTransportResponse",
  CKTapThreadState.finished: "finished",
  CKTapThreadState.canceled: "canceled",
  CKTapThreadState.failed: "failed",
  CKTapThreadState.invalidCardProduced: "invalidCardProduced",
  CKTapThreadState.tapProtocolError: "tapProtocolError",
  CKTapThreadState.timeout: "timeout",
  CKTapThreadState.transportException: "transportException",
};
