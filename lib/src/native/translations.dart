import 'dart:ffi';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';

import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/src/native/bindings.dart';
import 'package:cktap_protocol/src/native/library.dart';

Uint8List dartListFromCBinaryArray(CBinaryArray array,
    {bool freeArray = false}) {
  if (array.ptr.address != 0 && array.length > 0) {
    var list = Uint8List(array.length);
    list.setAll(0, array.ptr.asTypedList(array.length));

    if (freeArray) {
      nativeLibrary.Utility_freeBinaryArray(array);
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
      nativeLibrary.Utility_freeString(cString);
    }
    return dartString;
  }

  return "";
}

Slot getActiveSatscardSlotFrom(int handle, int type) {
  IntermediateSatscardSlot intermediary =
      nativeLibrary.Satscard_getActiveSlot(handle, type);
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

  nativeLibrary.Utility_freeIntermediateSatscardSlot(intermediary);
  return slot;
}

String getLiteralFromTapInterfaceErrorCode(int code) {
  return tapInterfaceErrorLiteralMap[code] ??
      "CKTapInterfaceErrorCode missing: $code";
}

String getLiteralFromTapProtoExceptionErrorCode(int code) {
  return tapProtoExceptionErrorLiteralMap[code] ??
      "CKTapProtoExceptionErrorCode missing: $code";
}

String getLiteralFromTapThreadState(int state) {
  return tapThreadStateLiteralMap[state] ?? "CKTapThreadState missing: $state";
}

CardType intToCardType(final int type) {
  switch (type) {
    case CKTapCardType.satscard:
      return CardType.satscard;
    case CKTapCardType.tapsigner:
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
