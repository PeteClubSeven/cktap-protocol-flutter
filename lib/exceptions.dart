import 'dart:core';

import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/src/error/validation.dart';
import 'package:cktap_protocol/src/native/bindings.dart';
import 'package:cktap_protocol/src/native/library.dart';
import 'package:cktap_protocol/src/native/translations.dart';
import 'package:cktap_protocol/transport.dart';
import 'package:nfc_manager/nfc_manager.dart';

/// The base class for user-facing exceptions thrown by this plugin
abstract class CKTapException implements Exception {}

/// Thrown when attempting to perform a handshake of an expected type but the
/// user has presented an invalid card
class InvalidCardException implements CKTapException {
  final CardType type;

  InvalidCardException(this.type);

  @override
  toString() =>
      "Expected $type but the user didn't present a card of that type";
}

/// Thrown when a given NfcTag doesn't support the required protocols for the
/// platform. Currently only IsoDep on Android and ISO7816 on iOS are supported
class NfcIncompatibilityException implements CKTapException {
  final NfcTag tag;

  NfcIncompatibilityException(this.tag);

  @override
  String toString() => "Given tag is incompatible with the CKTap plugin: $tag";
}

/// Thrown when attempts to communicate via NFC fail
class NfcCommunicationException implements CKTapException {
  final String message;
  final NfcTag tag;
  final NfcProtocol protocol;

  NfcCommunicationException(this.message, this.tag, this.protocol);

  @override
  String toString() =>
      "Failure to communicate via $protocol with the given tag (${tag.handle}): $message";
}

/// Thrown when an ongoing operation is canceled unexpectedly
class OperationCanceledException implements CKTapException {
  final String message;

  OperationCanceledException(this.message);

  @override
  String toString() => "A CKTapProtocol operation was canceled: $message";
}

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
