import 'cktap_protocol.dart';
import 'internal/utils.dart';

enum CardType {
  unknown,
  satscard,
  tapsigner,
}

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

  CKTapCard(this.handle, int type)
    : type = intToCardType(type),
      ident = 
}