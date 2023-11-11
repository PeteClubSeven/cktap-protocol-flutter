import 'dart:typed_data';

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

enum CardType {
  unknown,
  satscard,
  tapsigner,
}

class Satscard extends CKTapCard {
  final Slot activeSlot;
  final int activeSlotIndex;
  final int numSlots;
  final bool hasUnusedSlots;
  final bool isUsedUp;

  Satscard(SatscardConstructorParams params)
      : activeSlot = Slot(params.activeSlot),
        activeSlotIndex = params.activeSlotIndex,
        numSlots = params.numSlots,
        hasUnusedSlots = params.hasUnusedSlots > 0,
        isUsedUp = params.isUsedUp > 0,
        super(params.base);
}

class Slot {
  final int index;
  final SlotStatus status;
  final String address;

  // Requires the CVC to acquire
  final Uint8List privkey;
  final Uint8List pubkey;
  final Uint8List masterPK;
  final Uint8List chainCode;

  bool get isValid => index >= 0;

  Slot(SlotConstructorParams params)
      : index = params.index,
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

class Tapsigner extends CKTapCard {
  final int numberOfBackups;
  final String derivationPath;

  Tapsigner(TapsignerConstructorParams params)
      : numberOfBackups = params.numberOfBackups,
        derivationPath = dartStringFromCString(params.derivationPath),
        super(params.base);
}
