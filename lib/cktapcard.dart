import 'package:cktap_protocol/satscard.dart';
import 'package:cktap_protocol/src/cktap_implementation.dart';
import 'package:cktap_protocol/src/native/bindings.dart';
import 'package:cktap_protocol/src/native/translations.dart';
import 'package:cktap_protocol/tap_protocol.dart';
import 'package:cktap_protocol/tapsigner.dart';
import 'package:cktap_protocol/transport.dart';

export 'package:cktap_protocol/satscard.dart';
export 'package:cktap_protocol/tapsigner.dart';

abstract class CKTapCard {
  final int handle;
  final CardType type;
  final String ident;
  final String appletVersion;
  final int birthHeight;
  final bool isCertsChecked;
  final bool isTampered;
  final bool isTestnet;
  final bool needSetup;
  int authDelay;

  bool get isTapsigner => type == CardType.tapsigner;

  Satscard? toSatscard() => !isTapsigner ? this as Satscard : null;
  Tapsigner? toTapsigner() => isTapsigner ? this as Tapsigner : null;

  Future<WaitResponse> wait(Transport transport) {
    return CKTapImplementation.instance.cktapcardWait(transport, handle, type)
        .then((value) {
          authDelay = value.authDelay;
          return value;
    });
  }

  CKTapCard(final CKTapCardConstructorParams params)
      : handle = params.handle,
        type = intToCardType(params.type),
        ident = dartStringFromCString(params.ident),
        appletVersion = dartStringFromCString(params.appletVersion),
        authDelay = params.authDelay,
        birthHeight = params.birthHeight,
        isCertsChecked = params.isCertsChecked > 0,
        isTampered = params.isTampered > 0,
        isTestnet = params.isTestnet > 0,
        needSetup = params.needsSetup > 0;
}

enum CardType {
  unknown,
  satscard,
  tapsigner,
}

class WaitResponse {
  final bool success;
  final int authDelay;

  const WaitResponse(this.success, this.authDelay);
}