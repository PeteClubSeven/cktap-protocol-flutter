import 'dart:ffi';

import 'package:ffi/ffi.dart';

import 'generated_bindings.dart';
import '../cktapcard.dart';

String dartStringFromCString(Pointer<Ascii>)

CardType intToCardType(final int type) {
  switch (type)
  {
    case CKTapCardType.Satscard:
      return CardType.satscard;
    case CKTapCardType.Tapsigner:
      return CardType.tapsigner;
    default:
      return CardType.unknown;
  }
}