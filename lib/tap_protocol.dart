import 'dart:async';

import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/exceptions.dart';
import 'package:cktap_protocol/src/native/thread.dart';
import 'package:cktap_protocol/src/nfc_bridge.dart';
import 'package:nfc_manager/nfc_manager.dart';

/// Implements the basic functionality required to interact with Coinkite NFC
/// cards
class CKTapProtocol {
  /// An instance of the protocol
  static CKTapProtocol? _instance;

  /// Gets or creates the shared instance
  static CKTapProtocol get _internal => _instance ??= CKTapProtocol();

  /// Attempts to communicate with the given NFC device to determine if it is a
  /// Coinkite NFC card (e.g. a Satscard or Tapsigner) and returns it
  static Future<CKTapCard> createCKTapCard(NfcTag tag) async {
    return await _internal._createCKTapCard(tag);
  }

  /// Performs a preliminary check to see if the given tag is potentially a
  /// compatible NFC card. This can only be confirmed by communicating via NFC
  /// with the cards
  static bool isLikelyCoinkiteCard(NfcTag tag) {
    final ndef = Ndef.from(tag);
    if (ndef != null && ndef.cachedMessage != null) {
      for (final record in ndef.cachedMessage!.records) {
        final payload = String.fromCharCodes(record.payload);
        if (isLikelySatscard(payload) || isLikelyTapsigner(payload)) {
          return true;
        }
      }
    }
    return false;
  }

  /// Checks if the given payload is that of a Satscard. The authenticity of an
  /// NFC device can only be confirmed by communicating via NFC
  static bool isLikelySatscard(String ndefRecordPayload) {
    return ndefRecordPayload.startsWith("satscard.com/start", 1);
  }

  /// Checks if the given payload is that of a Tapsigner. The authenticity of an
  /// NFC device can only be confirmed by communicating via NFC
  static bool isLikelyTapsigner(String ndefRecordPayload) {
    return ndefRecordPayload.startsWith("tapsigner.com/start", 1);
  }

  Future<CKTapCard> _createCKTapCard(NfcTag tag) async {
    var nfc = NfcBridge.fromTag(tag);
    try {
      await prepareNativeThread();
      await prepareForCardHandshake();
      await processTransportRequests(nfc);
      return await finalizeCardCreation();
    } on NfcCommunicationException catch (e) {
      // TODO: Cancel thread if its still operating
      rethrow;
    } catch (e, s) {
      print(e);
      print(s);
    }

    return Future.error(-6);
  }
}
