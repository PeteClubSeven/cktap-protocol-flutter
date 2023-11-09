import 'dart:async';

import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/exceptions.dart';
import 'package:cktap_protocol/src/error/validation.dart';
import 'package:cktap_protocol/src/native/bindings.dart';
import 'package:cktap_protocol/src/native/library.dart';
import 'package:cktap_protocol/src/native/thread.dart';
import 'package:cktap_protocol/src/nfc_bridge.dart';
import 'package:nfc_manager/nfc_manager.dart';

/// Interfaces with a native implementation of the tap protocol to perform
/// various operations on Coinkite NFC devices
class CKTapImplementation {
  /// A copy of bindings to the native C++ library
  final NativeBindings bindings;

  /// The most recent cleanup operation
  Future? _cleanupFuture;

  /// A singleton for use across the plugin
  static CKTapImplementation? _staticInstance;

  /// Will initialize the library with the given bindings
  CKTapImplementation(this.bindings) {
    ensureSuccessful(bindings.Core_initializeLibrary());
  }

  /// Loads the required native DLLs initializes the library
  factory CKTapImplementation._initialize() {
    const List<String> dependencies = ['tap-protocol'];
    const String pluginLibName = 'cktap_protocol';

    for (final libName in dependencies) {
      loadLibrary(libName);
    }

    final bindings = NativeBindings(loadLibrary(pluginLibName));
    return CKTapImplementation(bindings);
  }

  /// Initializes the shared instance and thus the library itself
  static CKTapImplementation get instance =>
      _staticInstance ??= CKTapImplementation._initialize();

  Future<CKTapCard> readCard(NfcTag tag, String spendCode) async {
    var nfc = NfcBridge.fromTag(tag);
    _awaitCleanup();

    try {
      await prepareNativeThread();
      await prepareForCardHandshake();
      await processTransportRequests(nfc);
      return await finalizeCardCreation();
    } on NfcCommunicationException catch (_) {
      _cancelOperation();
      rethrow;
    } catch (e, s) {
      _cancelOperation();
      print(e);
      print(s);
      rethrow;
    }
  }

  Future<void> _awaitCleanup() async {
    if (_cleanupFuture == null) {
      return;
    }
    await _cleanupFuture;
    _cleanupFuture = null;
  }

  Future<void> _cancelOperation() {
    return _cleanupFuture = cancelNativeOperation()
        .timeout(const Duration(seconds: 2))
        .then((_) => _cleanupFuture = null)
        .onError((e, s) {
      print(e);
      print(s);
    });
  }
}
