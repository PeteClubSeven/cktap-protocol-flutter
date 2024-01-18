import 'dart:core';

import 'package:cktap_protocol/src/error/validation.dart';
import 'package:cktap_protocol/src/native/bindings.dart';
import 'package:cktap_protocol/src/native/library.dart';
import 'package:cktap_protocol/src/native/translations.dart';

/// The base class for user-facing exceptions thrown by this plugin
abstract class CKTapException implements Exception {}

/// Thrown when the given chain code is either not empty OR not a 64-character
/// hex code
class ChainCodeException implements CKTapException {
  final String chainCode;

  ChainCodeException(this.chainCode);

  @override
  String toString() =>
      "The given chain code either wasn't long enough or contained invalid characters: $chainCode";
}

/// Thrown when attempting to perform a handshake of an expected type but the
/// user has presented an invalid card
class InvalidCardException implements CKTapException {
  InvalidCardException();

  @override
  toString() => "User didn't present a card of the expected type";
}

/// Thrown when an ongoing operation is canceled unexpectedly
class OperationCanceledException implements CKTapException {
  final String message;

  OperationCanceledException(this.message);

  @override
  String toString() => "A CKTap operation was canceled: $message";
}

/// Thrown when the given spend code does not meet the requirements of being a
/// 6-digit numeric code
class SpendCodeException implements CKTapException {
  final String spendCode;

  const SpendCodeException(this.spendCode);

  @override
  String toString() =>
      "spendCode ($spendCode) given is not a 6-digit numeric code";
}

/// Generated named constants for each error code
typedef TapProtoExceptionCode = CKTapProtoExceptionErrorCode;

/// Thrown when the internal Nunchuk tap_protocol library throws an exception
class TapProtoException implements CKTapException {
  final int code;
  final String literal;
  final String message;

  TapProtoException._internal(this.code, this.literal, this.message);

  factory TapProtoException.fromCode(int errorCode, String message) {
    return TapProtoException._internal(errorCode,
        getLiteralFromTapProtoExceptionErrorCode(errorCode), message);
  }

  factory TapProtoException.fromException(final CKTapProtoException e) {
    final message = dartStringFromCString(e.message);
    return TapProtoException.fromCode(e.code, message);
  }

  factory TapProtoException.fromNative() {
    ensureNativeThreadState(CKTapThreadState.tapProtocolError);
    final exception = nativeLibrary.Core_getTapProtoException();
    final message = dartStringFromCString(exception.message);
    nativeLibrary.Utility_freeCKTapProtoException(exception);
    return TapProtoException.fromCode(exception.code, message);
  }

  @override
  String toString() => "$literal ($code): $message";
}
