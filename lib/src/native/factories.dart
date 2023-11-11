import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/src/error/types.dart';
import 'package:cktap_protocol/src/native/bindings.dart';
import 'package:cktap_protocol/src/native/library.dart';
import 'package:cktap_protocol/src/native/translations.dart';

CKTapCard makeCardFromHandle(CKTapCardHandle handle) =>
    makeCardOfType(handle.index, intToCardType(handle.type));

CKTapCard makeCardOfType(int handle, CardType type) {
  switch (type) {
    case CardType.satscard:
      return makeSatscardFromHandle(handle);
    case CardType.tapsigner:
      return makeTapsignerFromHandle(handle);
    default:
      throw InvalidCardTypeError(handle, type);
  }
}

Satscard makeSatscardFromHandle(int handle) {
  final params = nativeLibrary.Satscard_createConstructorParams(handle);
  try {
    if (params.errorCode == CKTapInterfaceErrorCode.unknownSatscardHandle) {
      throw InvalidCardTypeError(handle, CardType.satscard);
    }
    var card = Satscard(params);
    return card;
  } finally {
    nativeLibrary.Utility_freeSatscardConstructorParams(params);
  }
}

Tapsigner makeTapsignerFromHandle(int handle) {
  final params = nativeLibrary.Tapsigner_createConstructorParams(handle);
  try {
    if (params.errorCode == CKTapInterfaceErrorCode.unknownTapsignerHandle) {
      throw InvalidCardTypeError(handle, CardType.tapsigner);
    }
    var card = Tapsigner(params);
    return card;
  } finally {
    nativeLibrary.Utility_freeTapsignerConstructorParams(params);
  }
}
