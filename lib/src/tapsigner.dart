import 'cktap_protocol.dart';
import 'cktapcard.dart';
import 'internal/utils.dart';

class Tapsigner extends CKTapCard {
  final int numberOfBackups;
  final String derivationPath;

  Tapsigner(handle, type)
      : numberOfBackups = bindings.Tapsigner_GetNumberOfBackups(handle, type),
        derivationPath = dartStringFromCString(
            bindings.Tapsigner_GetDerivationPath(handle, type)),
        super(handle, type);
}
