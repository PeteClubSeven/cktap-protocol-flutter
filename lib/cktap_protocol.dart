import 'dart:async';

import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/src/implementation.dart';
import 'package:cktap_transport/cktap_transport.dart';

/// Implements the basic functionality required to interact with Coinkite NFC
/// cards
class CKTap {
  /// Calling this is optional as the library is lazily loaded when needed.
  /// However, if you wish to control exactly when the library initializes you
  /// can simple call this function. Subsequent calls won't do anything
  static void initialize() {
    Implementation.instance;
  }

  /// Checks if the given payload is that of a Satscard. The authenticity of an
  /// NFC device can only be confirmed by communicating via NFC
  static bool isLikelySatscard(String ndefRecordPayload) => ndefRecordPayload
      .startsWith(RegExp(r"(satscard|getsatscard)\.com/start"), 1);

  /// Checks if the given payload is that of a Tapsigner. The authenticity of an
  /// NFC device can only be confirmed by communicating via NFC
  static bool isLikelyTapsigner(String ndefRecordPayload) =>
      ndefRecordPayload.startsWith("tapsigner.com/start", 1);

  /// Attempts to communicate with the given NFC device to determine if it is a
  /// Coinkite NFC card (e.g. a Satscard or Tapsigner) and returns it. If you
  /// are expecting a certain card type you can specify it. This will be faster
  /// if the given tag is a match however it will fail entirely if the wrong
  /// card type is given, e.g. Tapsigner instead of a Satscard
  static Future<CKTapCard> readCard(Transport transport,
          {CardType type = CardType.unknown}) =>
      Implementation.instance.readCard(transport, type);
}
