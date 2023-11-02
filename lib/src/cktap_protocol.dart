import 'dart:async';
import 'dart:ffi';

import 'dart:typed_data';
import 'package:cktap_protocol/src/cktapcard.dart';
import 'package:nfc_manager/nfc_manager.dart';
import 'package:nfc_manager/platform_tags.dart';

import 'internal/library.dart';
import 'internal/native_thread.dart';
import 'satscard.dart';
import 'tapsigner.dart';

/// Implements the basic functionality required to interact with Coinkite NFC
/// cards
class CKTapCardProtocol {
  /// An instance of the protocol
  static CKTapCardProtocol? _instance;

  /// Gets or creates the shared instance
  static CKTapCardProtocol get instance => _instance ??= CKTapCardProtocol();

  /// Performs a preliminary check to see if the given tag is potentially a
  /// compatible NFC card
  static bool isCoinkiteCard(NfcTag tag) {
    var ndef = Ndef.from(tag);
    if (ndef == null || ndef.cachedMessage == null) {
      return false;
    }

    for (var record in ndef.cachedMessage!.records) {
      var payload = String.fromCharCodes(record.payload);
      var isValid = isSatscard(payload) || isTapsigner(payload);
      if (isValid) {
        return true;
      }
    }

    return false;
  }

  /// Checks if the given payload is likely to be a Satscard
  static bool isSatscard(String ndefRecordPayload) {
    return ndefRecordPayload.startsWith("satscard.com/start", 1);
  }

  /// Checks if the given payload is likely to be a Tapsigner
  static bool isTapsigner(String ndefRecordPayload) {
    return ndefRecordPayload.startsWith("tapsigner.com/start", 1);
  }

  /// Attempts to communicate with the given NFC device to determine if it is a
  /// Coinkite NFC card (e.g. a Satscard or Tapsigner) and returns it
  Future<CKTapCard> createCKTapCard(NfcTag tag,
      {bool pollAllSatscardSlots = false}) async {
    // TODO: iOS support
    // This is Android-exclusive
    var isoDep = IsoDep.from(tag);
    if (isoDep == null) {
      return Future.error(-1);
    }

    var initResponse = nativeLibrary.Core_BeginInitialization();
    if (initResponse != CKTapInterfaceErrorCode.Success) {
      return Future.error(initResponse);
    }

    int threadState;
    do {
      threadState = nativeLibrary.Core_GetThreadState();

      // Allow the background thread to reach the desired state
      if (threadState != CKTapThreadState.TransportRequestReady) {
        await Future.delayed(const Duration(microseconds: 100));
        continue;
      }

      // Communicate any necessary data with the NFC card
      Uint8List transportRequest = getNativeTransportRequest();
      // TODO: iOS support
      var transportResponse = await isoDep.transceive(data: transportRequest);
      var errorCode = setNativeTransportResponse(transportResponse);
      if (errorCode != CKTapInterfaceErrorCode.Success) {
        return Future.error(errorCode);
      }
    } while (threadState != CKTapThreadState.Finished &&
        threadState != CKTapThreadState.Timeout);

    // Finalize the initialization of the card and prepare the result
    if (threadState == CKTapThreadState.Finished) {
      var response = nativeLibrary.Core_FinalizeRecentOperation();
      if (response.errorCode != CKTapInterfaceErrorCode.Success) {
        return Future.error(response.errorCode);
      }

      if (response.handle.type == CKTapCardType.Satscard) {
        var satscard = Satscard(response.handle.index, response.handle.type);
        print(satscard);
        return satscard;
      } else if (response.handle.type == CKTapCardType.Tapsigner) {
        var tapsigner = Tapsigner(response.handle.index, response.handle.type);
        print(tapsigner);
        return tapsigner;
      }
    }

    return Future.error(-6);
  }
}
