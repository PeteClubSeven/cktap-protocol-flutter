import 'dart:typed_data';

import 'package:cktap_protocol/src/cktap_implementation.dart';
import 'package:cktap_protocol/src/native/bindings.dart';
import 'package:cktap_protocol/src/native/translations.dart';

abstract class CKTapCard {
  final int handle;
  final CardType type;
  final String ident;
  final String appletVersion;
  final int authDelay;
  final int birthHeight;
  final bool isCertsChecked;
  final bool isTampered;
  final bool isTestnet;
  final bool needSetup;

  bool get isTapsigner => type == CardType.tapsigner;

  Satscard? toSatscard() => !isTapsigner ? this as Satscard : null;
  Tapsigner? toTapsigner() => isTapsigner ? this as Tapsigner : null;

  CKTapCard(final CKTapCardConstructorParams params)
      : handle = params.handle,
        type = intToCardType(params.type),
        ident = dartStringFromCString(params.ident),
        appletVersion = dartStringFromCString(params.appletVersion),
        authDelay = params.authDelay,
        birthHeight = params.birthHeight,
        isCertsChecked = params.isCertsChecked > 0,
        isTampered = params.isTampered > 0,
        isTestnet = params.isTestnet > 0,
        needSetup = params.needsSetup > 0;
}

class Satscard extends CKTapCard {
  final int activeSlotIndex;
  final int numSlots;
  final bool hasUnusedSlots;
  final bool isUsedUp;

  Future<Slot> getActiveSlot() {
    return CKTapImplementation.instance.satscardGetActiveSlot(handle);
  }

  Satscard(SatscardConstructorParams params)
      : activeSlotIndex = params.activeSlotIndex,
        numSlots = params.numSlots,
        hasUnusedSlots = params.hasUnusedSlots > 0,
        isUsedUp = params.isUsedUp > 0,
        super(params.base);
}

class Tapsigner extends CKTapCard {
  final int numberOfBackups;
  final String derivationPath;

  Tapsigner(TapsignerConstructorParams params)
      : numberOfBackups = params.numberOfBackups,
        derivationPath = dartStringFromCString(params.derivationPath),
        super(params.base);
}

enum CardType {
  unknown,
  satscard,
  tapsigner,
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

  Future<String> toWif() {
    return CKTapImplementation.instance.slotToWif(_owner, index);
  }

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
