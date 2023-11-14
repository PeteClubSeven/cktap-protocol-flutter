import 'dart:async';

import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/src/error/types.dart';
import 'package:cktap_protocol/src/error/validation.dart';
import 'package:cktap_protocol/src/native/bindings.dart';
import 'package:cktap_protocol/src/native/library.dart';
import 'package:cktap_protocol/src/native/thread.dart';
import 'package:cktap_protocol/src/native/translations.dart';
import 'package:cktap_protocol/transport.dart';

/// Interfaces with a native implementation of the tap protocol to perform
/// various operations on Coinkite NFC devices
class CKTapImplementation {
  /// A copy of bindings to the native C++ library
  final NativeBindings bindings;

  /// See [CKTapImplementation.performAsyncOperation]
  bool _isPerformingNativeAction = false;

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

  /// A naive approach to ensuring concurrent operations are not being
  /// performed. This normally wouldn't be necessary but because we have a
  /// native background thread we can only guarantee the library is free of race
  /// conditions by flagging when an async operation is in progress
  Future<T> performNativeOperation<T>(
      Future<T> Function(NativeBindings) action) async {
    return Future.sync(() async {
      if (_isPerformingNativeAction) {
        throw ProtocolConcurrencyError(
            "Attempt to perform a concurrent native action in CKTapProtocol");
      }

      // Cleanup is a special case where we can just safely wait for cleanup if
      // the user of the library tries performing a second action too quickly
      _awaitCleanup();
      try {
        _isPerformingNativeAction = true;
        return await action(bindings);
      } catch (e, s) {
        print(e);
        print(s);
        _cancelOperation();
        rethrow;
      } finally {
        _isPerformingNativeAction = false;
      }
    });
  }

  Future<WaitResponse> cktapcardWait(
      Transport transport, int handle, CardType type) {
    return _performAsyncCardOperation(handle, type, (bindings) {
      ensureSuccessful(bindings.CKTapCard_beginWait());
      return processTransportRequests(transport, type).then((_) {
        var response = bindings.CKTapCard_getWaitResponse();
        ensureStatus(response.status, free: true);
        return WaitResponse(response.success > 0, response.authDelay);
      });
    });
  }

  Future<CKTapCard> readCard(
      Transport transport, String spendCode, CardType type) async {
    return performNativeOperation((_) {
      prepareNativeThread();
      prepareForCardHandshake(type);
      return processTransportRequests(transport, type);
    }).then((_) => finalizeCardCreation());
  }

  Future<Slot> satscardGetActiveSlot(int satscard) async {
    return performNativeOperation((bindings) async {
      var response = bindings.Satscard_getActiveSlot(satscard);
      ensureStatus(response.status, free:true);
      return Slot(response.params);
    });
  }

  Future<String> slotToWif(int satscard, int slot) async {
    return performNativeOperation((bindings) async {
      var response = bindings.Satscard_slotToWif(satscard, slot);
      try {
        ensureStatus(response.status);
        return dartStringFromCString(response.wif);
      } finally {
        bindings.Utility_freeSlotToWifResponse(response);
      }
    });
  }

  Future<void> _awaitCleanup() async {
    if (_cleanupFuture == null) {
      return;
    }
    await _cleanupFuture;
    _cleanupFuture = null;
  }

  Future<void> _cancelOperation() {
    return _cleanupFuture = cancelNativeOperation().catchError((e, s) {
      print(e);
      print(s);
    }).whenComplete(() => _cleanupFuture = null);
  }

  Future<T> _performAsyncCardOperation<T>(int handle, CardType type,
      Future<T> Function(NativeBindings) action) async {
    return performNativeOperation((bindings) {
      prepareNativeThread();
      prepareForCardOperation(handle, type);
      return action(bindings);
    });
  }
}
