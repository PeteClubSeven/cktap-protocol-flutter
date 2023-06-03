import 'dart:ffi';
import 'dart:io';

export 'generated_bindings.dart';
import 'generated_bindings.dart';

/// The bindings to the native functions in [CKTapProtocolBindings] and its dependencies.
final CKTapProtocolBindings nativeLibrary = CKTapProtocolBindings(() {
  const List<String> dependencies = ['tap-protocol'];
  const String pluginLibName = 'cktap_protocol';

  for (final libName in dependencies) {
    _loadLibrary(libName);
  }

  return _loadLibrary(pluginLibName);
}());

/// Loads the library of the given name in the expected format for the current platform
DynamicLibrary _loadLibrary(final String libName) {
  if (Platform.isMacOS || Platform.isIOS) {
    return DynamicLibrary.open('$libName.framework/$libName');
  }
  if (Platform.isAndroid || Platform.isLinux) {
    return DynamicLibrary.open('lib$libName.so');
  }
  if (Platform.isWindows) {
    return DynamicLibrary.open('$libName.dll');
  }
  throw UnsupportedError(
      'Unknown platform (${Platform.operatingSystem}) for lib: $libName');
}
