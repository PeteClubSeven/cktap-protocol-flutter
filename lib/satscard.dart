import 'dart:typed_data';

import 'package:cktap_protocol/cktap.dart';
import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/src/implementation.dart';
import 'package:cktap_protocol/src/error/validation.dart';
import 'package:cktap_protocol/src/native/bindings.dart';
import 'package:cktap_protocol/src/native/translations.dart';
import 'package:cktap_protocol/transport.dart';

class Satscard extends CKTapCard {
  final int numSlots;

  int activeSlotIndex;
  bool hasUnusedSlots;
  bool isUsedUp;

  static Future<Satscard> fromTransport(Transport transport) =>
      CKTap.readCard(transport, type: CardType.satscard)
          .then((card) => card.toSatscard()!);

  /// Checks the certificate of the Satscard to ensure it's an authentic card.
  /// [isCertsChecked] will be updated based on the result
  Future<void> certificateCheck(Transport transport) => Implementation.instance
      .satscardCertificateCheck(transport, handle)
      .then((value) => isCertsChecked = value);

  /// Constructs and returns the active slot of the Satscard, if one exists. Any
  /// Satscard which hasn't been fully used up will have an active slot, however
  /// if [isUsedUp] is true then this will fail
  Future<Slot> getActiveSlot() =>
      Implementation.instance.satscardGetActiveSlot(handle);

  /// Requests the given slot from the Satscard. If the [spendCode] is provided
  /// then the private key will also be revealed for [SlotStatus.unsealed] slots
  Future<Slot> getSlot(Transport transport, int slot,
          {String spendCode = ""}) =>
      Implementation.instance
          .satscardGetSlot(transport, slot, spendCode, handle)
          .then((value) => _sync(value));

  /// Requests every slot from the Satscard. If the [spendCode] is provided then
  /// the private keys will also be available for [SlotStatus.unsealed] slots
  Future<List<Slot>> listSlots(Transport transport,
          {String spendCode = "", int limit = 10}) =>
      Implementation.instance
          .satscardListSlots(transport, spendCode, limit, handle)
          .then((value) => _sync(value));

  /// Attempts to initialize the next slot of the Satscard, revealing a new
  /// public key and making the next slot active. If [isUsedUp] this will fail.
  /// [chainCode] must either be empty (and will be generated by the library) or
  /// a 64-character (32-byte) hex string. [spendCode] must be a 6-digit numeric
  /// code
  Future<Slot> newSlot(Transport transport, String spendCode,
          {String chainCode = ""}) =>
      Implementation.instance
          .satscardNew(transport, spendCode, chainCode, handle)
          .then((value) => _sync(value));

  /// Attempts to unseal the current slot revealing the private key for the
  /// active slot. If [isUsedUp] is true this will fail. [spendCode] must be a
  /// 6-digit numeric code
  Future<Slot> unseal(Transport transport, String spendCode) =>
      Implementation.instance
          .satscardUnseal(transport, spendCode, handle)
          .then((value) => _sync(value));

  /// Used to construct a Satscard from native data
  Satscard(SatscardConstructorParams params)
      : activeSlotIndex = params.activeSlotIndex,
        numSlots = params.numSlots,
        hasUnusedSlots = params.hasUnusedSlots > 0,
        isUsedUp = params.isUsedUp > 0,
        super(params.base);

  /// Performs a quick sync of mutable fields with the native implementation
  Future<T> _sync<T>(T value) async =>
      Implementation.instance.performNativeOperation((b) async {
        final params = b.Satscard_createSyncParams(handle);
        try {
          ensureStatus(params.status);
          isCertsChecked = params.baseParams.isCertsChecked > 0;
          needSetup = params.baseParams.needSetup > 0;
          authDelay = params.baseParams.authDelay;
          activeSlotIndex = params.activeSlotIndex;
          hasUnusedSlots = params.hasUnusedSlots > 0;
          isUsedUp = params.isUsedUp > 0;
          return value;
        } finally {
          b.Utility_freeSatscardSyncParams(params);
        }
      });
}

class Slot {
  final int _owner;
  final int index;
  final SlotStatus status;
  final String address;

  // Requires the CVC to acquire
  final Uint8List privkey;
  final Uint8List pubkey;
  final Uint8List masterPK;
  final Uint8List chainCode;

  Future<String> toWif() => Implementation.instance.slotToWif(_owner, index);

  Slot(SlotConstructorParams params)
      : _owner = params.satscardHandle,
        index = params.index,
        status = intToSlotStatus(params.status),
        address = dartStringFromCString(params.address),
        privkey = dartListFromCBinaryArray(params.privkey),
        pubkey = dartListFromCBinaryArray(params.pubkey),
        masterPK = dartListFromCBinaryArray(params.masterPK),
        chainCode = dartListFromCBinaryArray(params.chainCode);
}

enum SlotStatus {
  unused,
  sealed,
  unsealed,
  invalid,
}
