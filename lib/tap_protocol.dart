import 'dart:async';

import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/src/cktap_implementation.dart';
import 'package:cktap_protocol/transport.dart';
import 'package:nfc_manager/nfc_manager.dart';

/// Implements the basic functionality required to interact with Coinkite NFC
/// cards
class CKTapProtocol {
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

  /// Attempts to communicate with the given NFC device to determine if it is a
  /// Coinkite NFC card (e.g. a Satscard or Tapsigner) and returns it. If you
  /// are expecting a certain card type you can specify it. This will be faster
  /// if the given tag is a match however it will fail entirely if the wrong
  /// card type is given, e.g. Tapsigner instead of a Satscard
  static Future<CKTapCard> readCard(Transport transport,
      {String spendCode = "", CardType type = CardType.unknown}) async {
    return await CKTapImplementation.instance
        .readCard(transport, spendCode, type);
  }

  /// Attempts to initialize the active slot of a Satscard. The spendCode must
  /// be given AND accurate. The chain code is optional but if provided it must
  /// be a 64 character hex string (upper case and lower case is supported).
  //static Future<Satscard> satscardNew()
}
