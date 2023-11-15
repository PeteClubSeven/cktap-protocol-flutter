import 'dart:ffi';
import 'dart:typed_data';

import 'package:cktap_protocol/exceptions.dart';
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
      nativeLibrary.Utility_freeCBinaryArray(array);
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
      nativeLibrary.Utility_freeCString(cString);
    }
    return dartString;
  }

  return "";
}

void freeCString(Pointer<Int8> ptr) {
  if (ptr != nullptr) {
    malloc.free(ptr);
  }
}

Pointer<Int8> allocNativeChainCode(final String code) {
  if (code.isEmpty) {
    return nullptr;
  }
  if (code.length == 64 && code.contains(RegExp(r"[a-fA-F0-9]{64}"))) {
    return code.toNativeUtf8().cast<Int8>();
  }

  throw ChainCodeException(code);
}

Pointer<Int8> allocNativeSpendCode(final String code, {bool optional = false}) {
  if (optional && code.isEmpty) {
    return nullptr;
  }
  if (code.length == 6 && code.contains(RegExp(r"\d{6}"))) {
    return code.toNativeUtf8().cast<Int8>();
  }
  throw SpendCodeException(code);
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
