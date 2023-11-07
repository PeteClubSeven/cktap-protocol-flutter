import 'dart:io';

import 'package:cktap_protocol/exceptions.dart';
import 'package:flutter/services.dart';
import 'package:nfc_manager/nfc_manager.dart';
import 'package:nfc_manager/platform_tags.dart';

/// Specifies the NFC specification used to communicate with a device
enum NfcProtocol {
  none,
  isoDep,
  iso7816,
}

/// Facilitates communication between the library and NFC devices. Currently
/// only supports Android and iOS (see cktapcard.h)
class NfcBridge {
  /// Original tag used to construct the bridge
  final NfcTag _tag;

  /// Android uses IsoDep
  final IsoDep? _isoDep;

  /// iOS uses ISO7816
  final Iso7816? _iso7816;

  /// Internal constructor
  NfcBridge._internal(this._tag, this._isoDep, this._iso7816);

  /// Constructs a valid bridge provided the given tag supports the correct
  /// formats, e.g. IsoDep on Android or ISO7816 on iOS
  factory NfcBridge.fromTag(NfcTag tag) {
    if (Platform.isAndroid) {
      var isoDep = IsoDep.from(tag);
      if (isoDep != null && isoDep.maxTransceiveLength > 0) {
        return NfcBridge._internal(tag, isoDep, null);
      }
    } else if (Platform.isIOS) {
      var iso7816 = Iso7816.from(tag);
      if (iso7816 != null) {
        return NfcBridge._internal(tag, null, iso7816);
      }
    }

    throw NfcIncompatibilityException(tag);
  }

  /// Gets the type of communication used for the current platform
  NfcProtocol getCommunicationType() {
    if (_isoDep != null) {
      return NfcProtocol.isoDep;
    } else if (_iso7816 != null) {
      return NfcProtocol.iso7816;
    }
    return NfcProtocol.none;
  }

  /// Sends raw data to the NFC device (if still available). Returns the raw
  /// response as bytes
  Future<Uint8List> send(Uint8List bytes) async {
    try {
      if (_isoDep != null) {
        return (await _isoDep!.transceive(data: bytes));
      } else if (_iso7816 != null) {
        return (await _iso7816!.sendCommandRaw(bytes)).payload;
      }
    } on PlatformException catch(e) {
      throw NfcCommunicationException(e.toString(), _tag, getCommunicationType());
    }

    throw UnsupportedError(
        "Attempt to send data to an NFC device on an unsupported platform");
  }
}
