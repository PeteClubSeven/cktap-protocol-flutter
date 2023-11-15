import 'dart:ffi';
import 'dart:io';

import 'package:cktap_protocol/src/cktap_implementation.dart';
import 'package:cktap_protocol/src/native/bindings.dart';

/// Gets an instance of the native library bindings
NativeBindings get nativeLibrary => CKTapImplementation.instance.bindings;

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
  CKTapInterfaceErrorCode.attemptToFinalizeActiveThread: "attemptToFinalizeActiveThread",
  CKTapInterfaceErrorCode.bindingNotImplemented: "bindingNotImplemented",
  CKTapInterfaceErrorCode.caughtTapProtocolException: "caughtTapProtocolException",
  CKTapInterfaceErrorCode.expectedSatscardButReceivedNothing: "expectedSatscardButReceivedNothing",
  CKTapInterfaceErrorCode.expectedTapsignerButReceivedNothing: "expectedTapsignerButReceivedNothing",
  CKTapInterfaceErrorCode.failedToPerformHandshake: "failedToPerformHandshake",
  CKTapInterfaceErrorCode.failedToRetrieveValueFromFuture: "failedToRetrieveValueFromFuture",
  CKTapInterfaceErrorCode.invalidCardDuringHandshake: "invalidCardDuringHandshake",
  CKTapInterfaceErrorCode.invalidCardOperation: "invalidCardOperation",
  CKTapInterfaceErrorCode.invalidHandlingOfCardDuringFinalization: "invalidHandlingOfCardDuringFinalization",
  CKTapInterfaceErrorCode.invalidResponseFromCardOperation: "invalidResponseFromCardOperation",
  CKTapInterfaceErrorCode.invalidThreadStateDuringTransportSignaling: "invalidThreadStateDuringTransportSignaling",
  CKTapInterfaceErrorCode.libraryNotInitialized: "libraryNotInitialized",
  CKTapInterfaceErrorCode.operationCanceled: "operationCanceled",
  CKTapInterfaceErrorCode.operationFailed: "operationFailed",
  CKTapInterfaceErrorCode.operationStillInProgress: "operationStillInProgress",
  CKTapInterfaceErrorCode.threadAlreadyInUse: "threadAlreadyInUse",
  CKTapInterfaceErrorCode.threadAllocationFailed: "threadAllocationFailed",
  CKTapInterfaceErrorCode.threadNotReadyForResponse: "threadNotReadyForResponse",
  CKTapInterfaceErrorCode.threadNotResetForHandshake: "threadNotResetForHandshake",
  CKTapInterfaceErrorCode.threadNotAwaitingCardOperation: "threadNotAwaitingCardOperation",
  CKTapInterfaceErrorCode.threadNotYetFinalized: "threadNotYetFinalized",
  CKTapInterfaceErrorCode.threadNotYetStarted: "threadNotYetStarted",
  CKTapInterfaceErrorCode.threadResponseFinalizationFailed: "threadResponseFinalizationFailed",
  CKTapInterfaceErrorCode.timeoutDuringTransport: "timeoutDuringTransport",
  CKTapInterfaceErrorCode.unableToFinalizeAsyncAction: "unableToFinalizeAsyncAction",
  CKTapInterfaceErrorCode.unexpectedExceptionWhenGettingCardOperationResult: "unexpectedExceptionWhenGettingCardOperationResult",
  CKTapInterfaceErrorCode.unexpectedExceptionWhenStartingCardOperation: "unexpectedExceptionWhenStartingCardOperation",
  CKTapInterfaceErrorCode.unexpectedStdException: "unexpectedStdException",
  CKTapInterfaceErrorCode.unknownErrorDuringAsyncOperation: "unknownErrorDuringAsyncOperation",
  CKTapInterfaceErrorCode.unknownErrorDuringHandshake: "unknownErrorDuringHandshake",
  CKTapInterfaceErrorCode.unknownErrorDuringTapProtocolFunction: "unknownErrorDuringTapProtocolFunction",
  CKTapInterfaceErrorCode.unknownSatscardHandle: "unknownSatscardHandle",
  CKTapInterfaceErrorCode.unknownSlotForGivenSatscardHandle: "unknownSlotForGivenSatscardHandle",
  CKTapInterfaceErrorCode.unknownTapsignerHandle: "unknownTapsignerHandle",
};

/// Maps Nunchuk tap_protocol error code numbers to a string-readable version
final Map<int, String> tapProtoExceptionErrorLiteralMap = {
  CKTapProtoExceptionErrorCode.INVALID_DEVICE: "tap_protocol::TapProtoException::INVALID_DEVICE",
  CKTapProtoExceptionErrorCode.UNLUCKY_NUMBER: "tap_protocol::TapProtoException::UNLUCKY_NUMBER",
  CKTapProtoExceptionErrorCode.BAD_ARGUMENTS: "tap_protocol::TapProtoException::BAD_ARGUMENTS",
  CKTapProtoExceptionErrorCode.BAD_AUTH: "tap_protocol::TapProtoException::BAD_AUTH",
  CKTapProtoExceptionErrorCode.NEED_AUTH: "tap_protocol::TapProtoException::NEED_AUTH",
  CKTapProtoExceptionErrorCode.UNKNOW_COMMAND: "tap_protocol::TapProtoException::UNKNOW_COMMAND",
  CKTapProtoExceptionErrorCode.INVALID_COMMAND: "tap_protocol::TapProtoException::INVALID_COMMAND",
  CKTapProtoExceptionErrorCode.INVALID_STATE: "tap_protocol::TapProtoException::INVALID_STATE",
  CKTapProtoExceptionErrorCode.WEAK_NONCE: "tap_protocol::TapProtoException::WEAK_NONCE",
  CKTapProtoExceptionErrorCode.BAD_CBOR: "tap_protocol::TapProtoException::BAD_CBOR",
  CKTapProtoExceptionErrorCode.BACKUP_FIRST: "tap_protocol::TapProtoException::BACKUP_FIRST",
  CKTapProtoExceptionErrorCode.RATE_LIMIT: "tap_protocol::TapProtoException::RATE_LIMIT",
  CKTapProtoExceptionErrorCode.DEFAULT_ERROR: "tap_protocol::TapProtoException::DEFAULT_ERROR",
  CKTapProtoExceptionErrorCode.MESSAGE_TOO_LONG: "tap_protocol::TapProtoException::MESSAGE_TOO_LONG",
  CKTapProtoExceptionErrorCode.MISSING_KEY: "tap_protocol::TapProtoException::MISSING_KEY",
  CKTapProtoExceptionErrorCode.ISO_SELECT_FAIL: "tap_protocol::TapProtoException::ISO_SELECT_FAIL",
  CKTapProtoExceptionErrorCode.SW_FAIL: "tap_protocol::TapProtoException::SW_FAIL",
  CKTapProtoExceptionErrorCode.INVALID_CVC_LENGTH: "tap_protocol::TapProtoException::INVALID_CVC_LENGTH",
  CKTapProtoExceptionErrorCode.PICK_KEY_PAIR_FAIL: "tap_protocol::TapProtoException::PICK_KEY_PAIR_FAIL",
  CKTapProtoExceptionErrorCode.ECDH_FAIL: "tap_protocol::TapProtoException::ECDH_FAIL",
  CKTapProtoExceptionErrorCode.XCVC_FAIL: "tap_protocol::TapProtoException::XCVC_FAIL",
  CKTapProtoExceptionErrorCode.UNKNOW_PROTO_VERSION: "tap_protocol::TapProtoException::UNKNOW_PROTO_VERSION",
  CKTapProtoExceptionErrorCode.INVALID_PUBKEY_LENGTH: "tap_protocol::TapProtoException::INVALID_PUBKEY_LENGTH",
  CKTapProtoExceptionErrorCode.NO_PRIVATE_KEY_PICKED: "tap_protocol::TapProtoException::NO_PRIVATE_KEY_PICKED",
  CKTapProtoExceptionErrorCode.MALFORMED_BIP32_PATH: "tap_protocol::TapProtoException::MALFORMED_BIP32_PATH",
  CKTapProtoExceptionErrorCode.INVALID_HASH_LENGTH: "tap_protocol::TapProtoException::INVALID_HASH_LENGTH",
  CKTapProtoExceptionErrorCode.SIG_VERIFY_ERROR: "tap_protocol::TapProtoException::SIG_VERIFY_ERROR",
  CKTapProtoExceptionErrorCode.INVALID_DIGEST_LENGTH: "tap_protocol::TapProtoException::INVALID_DIGEST_LENGTH",
  CKTapProtoExceptionErrorCode.INVALID_PATH_LENGTH: "tap_protocol::TapProtoException::INVALID_PATH_LENGTH",
  CKTapProtoExceptionErrorCode.SERIALIZE_ERROR: "tap_protocol::TapProtoException::SERIALIZE_ERROR",
  CKTapProtoExceptionErrorCode.EXCEEDED_RETRY: "tap_protocol::TapProtoException::EXCEEDED_RETRY",
  CKTapProtoExceptionErrorCode.INVALID_CARD: "tap_protocol::TapProtoException::INVALID_CARD",
  CKTapProtoExceptionErrorCode.SIGN_ERROR: "tap_protocol::TapProtoException::SIGN_ERROR",
  CKTapProtoExceptionErrorCode.SIG_TO_PUBKEY_FAIL: "tap_protocol::TapProtoException::SIG_TO_PUBKEY_FAIL",
  CKTapProtoExceptionErrorCode.PSBT_PARSE_ERROR: "tap_protocol::TapProtoException::PSBT_PARSE_ERROR",
  CKTapProtoExceptionErrorCode.PSBT_INVALID: "tap_protocol::TapProtoException::PSBT_INVALID",
  CKTapProtoExceptionErrorCode.INVALID_ADDRESS_TYPE: "tap_protocol::TapProtoException::INVALID_ADDRESS_TYPE",
  CKTapProtoExceptionErrorCode.INVALID_BACKUP_KEY: "tap_protocol::TapProtoException::INVALID_BACKUP_KEY",
  CKTapProtoExceptionErrorCode.INVALID_PUBKEY: "tap_protocol::TapProtoException::INVALID_PUBKEY",
  CKTapProtoExceptionErrorCode.INVALID_PRIVKEY: "tap_protocol::TapProtoException::INVALID_PRIVKEY",
  CKTapProtoExceptionErrorCode.INVALID_SLOT: "tap_protocol::TapProtoException::INVALID_SLOT",
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
