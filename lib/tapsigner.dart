import 'package:cktap_protocol/cktap_protocol.dart';
import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/src/native/bindings.dart';
import 'package:cktap_protocol/src/native/translations.dart';
import 'package:cktap_protocol/transport.dart';

class Tapsigner extends CKTapCard {
  final int numberOfBackups;
  final String derivationPath;

  static Future<Tapsigner> fromTransport(Transport transport) =>
      CKTapProtocol.readCard(transport, type: CardType.tapsigner)
          .then((card) => card.toTapsigner()!);

  Tapsigner(TapsignerConstructorParams params)
      : numberOfBackups = params.numberOfBackups,
        derivationPath = dartStringFromCString(params.derivationPath),
        super(params.base);
}
