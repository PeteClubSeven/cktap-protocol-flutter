import 'dart:typed_data';

import 'package:cktap_protocol/src/native/library.dart';
import 'package:cktap_protocol/src/native/translations.dart';

abstract class CKTapCard {
  final int handle;
  final CardType type;
  final String ident;
  final String appletVersion;
  final int birthHeight;
  final bool isTestnet;
  final int authDelay;
  final bool isTampered;
  final bool isCertsChecked;
  final bool needSetup;

  bool get isTapsigner => type == CardType.tapsigner;

  Satscard? toSatscard() => !isTapsigner ? this as Satscard : null;

  Tapsigner? toTapsigner() => isTapsigner ? this as Tapsigner : null;

  CKTapCard(this.handle, int type)
      : type = intToCardType(type),
        ident = dartStringFromCString(
            nativeLibrary.CKTapCard_getIdentCString(handle, type),
            freeCString: true),
        appletVersion = dartStringFromCString(
            nativeLibrary.CKTapCard_getAppletVersionCString(handle, type),
            freeCString: true),
        birthHeight = nativeLibrary.CKTapCard_getBirthHeight(handle, type),
        isTestnet = nativeLibrary.CKTapCard_isTestnet(handle, type) > 0,
        authDelay = nativeLibrary.CKTapCard_getAuthDelay(handle, type),
        isTampered = nativeLibrary.CKTapCard_isTampered(handle, type) > 0,
        isCertsChecked =
            nativeLibrary.CKTapCard_isCertsChecked(handle, type) > 0,
        needSetup = nativeLibrary.CKTapCard_needSetup(handle, type) > 0;
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

  Satscard(handle, type)
      : activeSlot = getActiveSatscardSlotFrom(handle, type),
        activeSlotIndex =
            nativeLibrary.Satscard_getActiveSlotIndex(handle, type),
        numSlots = nativeLibrary.Satscard_getNumSlots(handle, type),
        hasUnusedSlots =
            nativeLibrary.Satscard_hasUnusedSlots(handle, type) > 0,
        isUsedUp = nativeLibrary.Satscard_isUsedUp(handle, type) > 0,
        super(handle, type);
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

  Slot(this.index, this.status, this.address)
      : privkey = Uint8List(0),
        pubkey = Uint8List(0),
        masterPK = Uint8List(0),
        chainCode = Uint8List(0);

  Slot.setAll(this.index, this.status, this.address, this.privkey, this.pubkey,
      this.masterPK, this.chainCode);

  Slot.invalid()
      : index = -1,
        status = SlotStatus.unused,
        address = "",
        privkey = Uint8List(0),
        pubkey = Uint8List(0),
        masterPK = Uint8List(0),
        chainCode = Uint8List(0);
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

  Tapsigner(handle, type)
      : numberOfBackups =
            nativeLibrary.Tapsigner_getNumberOfBackups(handle, type),
        derivationPath = dartStringFromCString(
            nativeLibrary.Tapsigner_getDerivationPath(handle, type)),
        super(handle, type);
}
