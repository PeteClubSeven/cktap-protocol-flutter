import 'dart:async';
import 'dart:ffi';

import 'package:cktap_protocol/src/cktapcard.dart';
import 'package:nfc_manager/nfc_manager.dart';
import 'package:nfc_manager/platform_tags.dart';

import 'internal/library.dart';
import 'satscard.dart';
import 'tapsigner.dart';

/// Implements the basic functionality required to interact with Coinkite NFC cards
class CKTapCardProtocol {
  /// An instance of the protocol
  static CKTapCardProtocol? _instance;

  /// Gets or creates the shared instance
  static CKTapCardProtocol get instance => _instance ??= CKTapCardProtocol();

  /// Performs a preliminary check to see if the given tag is potentially a compatible NFC card
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

  static bool isSatscard(String ndefRecordPayload) {
    return ndefRecordPayload.startsWith("satscard.com/start", 1);
  }

  static bool isTapsigner(String ndefRecordPayload) {
    return ndefRecordPayload.startsWith("tapsigner.com/start", 1);
  }

  Future<CKTapCard> createCKTapCard(NfcTag tag,
      {bool pollAllSatscardSlots = false}) async {
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
      switch (threadState) {
        case CKTapThreadState.AwaitingTransportRequest:
        case CKTapThreadState.TransportResponseReady:
        case CKTapThreadState.ProcessingTransportResponse:
          await Future.delayed(const Duration(microseconds: 100));
          break;
        case CKTapThreadState.TransportRequestReady:
          var pointer = nativeLibrary.Core_GetTransportRequestPointer();
          var length = nativeLibrary.Core_GetTransportRequestLength();
          if (pointer.address == 0) {
            return Future.error(-2);
          }
          if (length == 0) {
            return Future.error(-3);
          }

          var transportRequest = pointer.asTypedList(length);
          await isoDep.transceive(data: transportRequest).then((value) {
            Pointer<Uint8> allocation =
                nativeLibrary.Core_AllocateTransportResponseBuffer(
                    value.length);
            if (allocation.address == 0) {
              return Future.error(-4);
            }

            var transportResponse = allocation.asTypedList(value.length);
            transportResponse.setAll(0, value);

            var errorCode = nativeLibrary.Core_FinalizeTransportResponse();
            if (errorCode != CKTapInterfaceErrorCode.Success) {
              return Future.error(errorCode);
            }

            return 1;
          }).catchError((err) {
            print('Error: $err');
            return -5;
          }, test: (error) {
            print(error);
            return error is int && error >= 400;
          });

          break;
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
