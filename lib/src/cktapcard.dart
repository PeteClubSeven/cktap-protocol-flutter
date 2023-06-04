import 'internal/library.dart';
import 'internal/utils.dart';
import 'satscard.dart';
import 'tapsigner.dart';

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
            nativeLibrary.CKTapCard_GetIdentCString(handle, type),
            freeCString: true),
        appletVersion = dartStringFromCString(
            nativeLibrary.CKTapCard_GetAppletVersionCString(handle, type),
            freeCString: true),
        birthHeight = nativeLibrary.CKTapCard_GetBirthHeight(handle, type),
        isTestnet = nativeLibrary.CKTapCard_IsTestnet(handle, type) > 0,
        authDelay = nativeLibrary.CKTapCard_GetAuthDelay(handle, type),
        isTampered = nativeLibrary.CKTapCard_IsTampered(handle, type) > 0,
        isCertsChecked =
            nativeLibrary.CKTapCard_IsCertsChecked(handle, type) > 0,
        needSetup = nativeLibrary.CKTapCard_NeedSetup(handle, type) > 0;
}

enum CardType {
  unknown,
  satscard,
  tapsigner,
}
