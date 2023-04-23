import 'cktap_protocol.dart';
import 'satscard.dart';
import 'tapsigner.dart';
import 'internal/utils.dart';

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
            bindings.CKTapCard_GetIdentCString(handle, type),
            freeCString: true),
        appletVersion = dartStringFromCString(
            bindings.CKTapCard_GetAppletVersionCString(handle, type),
            freeCString: true),
        birthHeight = bindings.CKTapCard_GetBirthHeight(handle, type),
        isTestnet = bindings.CKTapCard_IsTestnet(handle, type) > 0,
        authDelay = bindings.CKTapCard_GetAuthDelay(handle, type),
        isTampered = bindings.CKTapCard_IsTampered(handle, type) > 0,
        isCertsChecked = bindings.CKTapCard_IsCertsChecked(handle, type) > 0,
        needSetup = bindings.CKTapCard_NeedSetup(handle, type) > 0;
}

enum CardType {
  unknown,
  satscard,
  tapsigner,
}
