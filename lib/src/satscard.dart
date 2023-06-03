import 'dart:typed_data';

import 'cktapcard.dart';
import 'internal/library.dart';
import 'internal/utils.dart';

class Satscard extends CKTapCard {
  final Slot activeSlot;
  final int activeSlotIndex;
  final int numSlots;
  final bool hasUnusedSlots;
  final bool isUsedUp;

  Satscard(handle, type)
      : activeSlot = getActiveSatscardSlotFrom(handle, type),
        activeSlotIndex =
            nativeLibrary.Satscard_GetActiveSlotIndex(handle, type),
        numSlots = nativeLibrary.Satscard_GetNumSlots(handle, type),
        hasUnusedSlots =
            nativeLibrary.Satscard_HasUnusedSlots(handle, type) > 0,
        isUsedUp = nativeLibrary.Satscard_IsUsedUp(handle, type) > 0,
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
