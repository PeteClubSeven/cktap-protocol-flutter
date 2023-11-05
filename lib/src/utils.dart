import 'dart:ffi';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';

import '../cktapcard.dart';
import 'native_library.dart';

Uint8List dartListFromCBinaryArray(CBinaryArray array,
    {bool freeArray = false}) {
  if (array.ptr.address != 0 && array.length > 0) {
    var list = Uint8List(array.length);
    list.setAll(0, array.ptr.asTypedList(array.length));

    if (freeArray) {
      nativeLibrary.Utility_FreeBinaryArray(array);
    }
    return list;
  }

  return Uint8List(0);
}

String dartStringFromCString(Pointer<Char> cString,
    {bool freeCString = false}) {
  if (cString.address != 0) {
    var utfString = cString.cast<Utf8>();
    var dartString = utfString.toDartString();

    // Ensure we clean up the string
    if (freeCString) {
      nativeLibrary.Utility_FreeString(cString);
    }
    return dartString;
  }

  return "";
}

Slot getActiveSatscardSlotFrom(int handle, int type) {
  IntermediateSatscardSlot intermediary =
      nativeLibrary.Satscard_GetActiveSlot(handle, type);
  if (intermediary.index < 0) {
    return Slot.invalid();
  }

  var slot = Slot.setAll(
      intermediary.index,
      intToSlotStatus(intermediary.status),
      dartStringFromCString(intermediary.address),
      dartListFromCBinaryArray(intermediary.privkey),
      dartListFromCBinaryArray(intermediary.pubkey),
      dartListFromCBinaryArray(intermediary.masterPK),
      dartListFromCBinaryArray(intermediary.chainCode));

  nativeLibrary.Utility_FreeIntermediateSatscardSlot(intermediary);
  return slot;
}

CardType intToCardType(final int type) {
  switch (type) {
    case CKTapCardType.Satscard:
      return CardType.satscard;
    case CKTapCardType.Tapsigner:
      return CardType.tapsigner;
    default:
      return CardType.unknown;
  }
}

SlotStatus intToSlotStatus(final int status) {
  switch (status) {
    case CKTapSatscardSlotStatus.UNUSED:
      return SlotStatus.unused;
    case CKTapSatscardSlotStatus.SEALED:
      return SlotStatus.sealed;
    case CKTapSatscardSlotStatus.UNSEALED:
      return SlotStatus.unsealed;
    default:
      return SlotStatus.invalid;
  }
}
