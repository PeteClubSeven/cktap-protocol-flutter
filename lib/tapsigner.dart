import 'package:cktap_protocol/cktap_protocol.dart';
import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/src/cktap_implementation.dart';
import 'package:cktap_protocol/src/error/validation.dart';
import 'package:cktap_protocol/src/native/bindings.dart';
import 'package:cktap_protocol/src/native/translations.dart';
import 'package:cktap_protocol/transport.dart';

class Tapsigner extends CKTapCard {
  int numberOfBackups;
  String derivationPath;

  static Future<Tapsigner> fromTransport(Transport transport) =>
      CKTapProtocol.readCard(transport, type: CardType.tapsigner)
          .then((card) => card.toTapsigner()!);

  /// Used to construct a Tapsigner from native data
  Tapsigner(TapsignerConstructorParams params)
      : numberOfBackups = params.numberOfBackups,
        derivationPath = dartStringFromCString(params.derivationPath),
        super(params.base);

  /// Performs a quick sync of mutable fields with the native implementation
  Future<T> _sync<T>(T value) async =>
      CKTapImplementation.instance.performNativeOperation((b) async {
        final params = b.Tapsigner_createSyncParams(handle);
        try {
          ensureStatus(params.status);
          isCertsChecked = params.baseParams.isCertsChecked > 0;
          needSetup = params.baseParams.needSetup > 0;
          authDelay = params.baseParams.authDelay;
          numberOfBackups = params.numberOfBackups;
          derivationPath = dartStringFromCString(params.derivationPath);
          return value;
        } finally {
          b.Utility_freeTapsignerSyncParams(params);
        }
      });
}
