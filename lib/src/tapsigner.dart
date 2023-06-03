import 'cktapcard.dart';
import 'internal/library.dart';
import 'internal/utils.dart';

class Tapsigner extends CKTapCard {
  final int numberOfBackups;
  final String derivationPath;

  Tapsigner(handle, type)
      : numberOfBackups =
            nativeLibrary.Tapsigner_GetNumberOfBackups(handle, type),
        derivationPath = dartStringFromCString(
            nativeLibrary.Tapsigner_GetDerivationPath(handle, type)),
        super(handle, type);
}
