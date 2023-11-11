import 'dart:core';

import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/src/native/translations.dart';

/// A base class for library-facing exceptions which MUST be handled internally
abstract class CKTapInternalException implements Exception {}

/// A base class for library errors which should never happen
abstract class CKTapError extends Error {}

class InvalidCardTypeError extends CKTapError {
  final int handle;
  final CardType type;

  InvalidCardTypeError(this.handle, this.type);

  @override
  String toString() =>
      "Attempt to use an invalid card type ($type) with handle: $handle";
}

/// Thrown when the native thread is in an invalid/unusable state
class InvalidThreadStateError extends CKTapError {
  final int expectedState;
  final int foundState;

  InvalidThreadStateError(this.expectedState, this.foundState);

  @override
  String toString() {
    return "Native thread in unexpected state, expected ${getLiteralFromTapThreadState(expectedState)} and found ${getLiteralFromTapThreadState(foundState)}";
  }
}

/// Thrown when trying to do multiple operations at once
class ProtocolConcurrencyError extends CKTapError {
  final String message;

  ProtocolConcurrencyError(this.message);

  @override
  String toString() => "Parallel CKTap operation failed: $message";
}

/// Thrown when encountering an interface error unexpectedly
class TapInterfaceError extends CKTapError {
  final int code;
  final String literal;

  TapInterfaceError._internal(this.code, this.literal);

  factory TapInterfaceError.fromCode(int errorCode) {
    return TapInterfaceError._internal(
        errorCode, getLiteralFromTapInterfaceErrorCode(errorCode));
  }

  @override
  String toString() =>
      "An internal error occurred in the CKTap library, code $code: $literal";
}
