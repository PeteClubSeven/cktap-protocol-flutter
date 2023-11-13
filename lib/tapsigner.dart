import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/src/native/bindings.dart';
import 'package:cktap_protocol/src/native/translations.dart';

class Tapsigner extends CKTapCard {
  final int numberOfBackups;
  final String derivationPath;

  Tapsigner(TapsignerConstructorParams params)
      : numberOfBackups = params.numberOfBackups,
        derivationPath = dartStringFromCString(params.derivationPath),
        super(params.base);
}