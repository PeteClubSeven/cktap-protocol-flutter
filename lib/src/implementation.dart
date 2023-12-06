import 'dart:async';
import 'dart:ffi';

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
class Implementation {
  /// A copy of bindings to the native C++ library
  final NativeBindings bindings;

  /// See [Implementation.performAsyncOperation]
  bool _isPerformingNativeAction = false;

  /// The most recent cleanup operation
  Future? _cleanupFuture;

  /// A singleton for use across the plugin
  static Implementation? _staticInstance;

  /// Initializes the shared instance and thus the library itself
  static Implementation get instance =>
      _staticInstance ??= Implementation._initialize();

  /// Will initialize the library with the given bindings
  Implementation(this.bindings) {
    ensure(bindings.Core_initializeLibrary());
  }

  /// Loads the required native DLLs initializes the library
  factory Implementation._initialize() {
    const List<String> dependencies = ['tap-protocol'];
    const String pluginLibName = 'cktap_protocol';

    for (final libName in dependencies) {
      loadLibrary(libName);
    }

    final bindings = NativeBindings(loadLibrary(pluginLibName));
    return Implementation(bindings);
  }

  Future<WaitResponse> cktapcardWait(Transport nfc, int handle, CardType type) {
    return _performAsyncCardOperation(handle, type, (lib) {
      ensure(lib.CKTapCard_beginWait());
      return processTransportRequests(nfc).then((_) {
        var response = lib.CKTapCard_getWaitResponse();
        ensureStatus(response.status, free: true);
        return WaitResponse(response.success > 0, response.authDelay);
      });
    });
  }

  Future<CKTapCard> readCard(Transport nfc, CardType type) {
    return performNativeOperation((_) {
      prepareNativeThread();
      prepareForCardHandshake(type);
      return processTransportRequests(nfc);
    }).then((_) => finalizeCardCreation());
  }

  Future<bool> satscardCertificateCheck(Transport nfc, int handle) {
    return _performAsyncCardOperation(handle, CardType.satscard, (lib) {
      ensure(lib.Satscard_beginCertificateCheck());
      return processTransportRequests(nfc).then((_) {
        var response = lib.Satscard_getCertificateCheckResponse();
        ensureStatus(response.status, free: true);
        return response.isCertsChecked > 0;
      });
    });
  }

  Future<Slot> satscardGetActiveSlot(int satscard) {
    return performNativeOperation((lib) async {
      var response = lib.Satscard_getActiveSlot(satscard);
      ensureStatus(response.status, free: true);
      return Slot(response.params);
    });
  }

  Future<Slot> satscardGetSlot(
      Transport nfc, int slot, String spend, int handle) {
    return Future.sync(() async {
      final nativeSpendCode = allocNativeSpendCode(spend, optional: true);
      try {
        return await _performAsyncCardOperation(handle, CardType.satscard,
            (lib) {
          ensure(lib.Satscard_beginGetSlot(slot, nativeSpendCode));
          return processTransportRequests(nfc).then((_) {
            var response = lib.Satscard_getGetSlotResponse(handle);
            try {
              ensureStatus(response.status);
              return Slot(response.params);
            } finally {
              lib.Utility_freeSatscardSlotResponse(response);
            }
          });
        });
      } finally {
        freeCString(nativeSpendCode);
      }
    });
  }

  Future<List<Slot>> satscardListSlots(
      Transport nfc, String spend, int limit, int handle) {
    return Future.sync(() async {
      final nativeSpendCode = allocNativeSpendCode(spend, optional: true);
      try {
        return await _performAsyncCardOperation(handle, CardType.satscard,
            (lib) {
          ensure(lib.Satscard_beginListSlots(nativeSpendCode, limit));
          return processTransportRequests(nfc).then((_) {
            var response = lib.Satscard_getListSlotsResponse(handle);
            try {
              ensureStatus(response.status);
              return List<Slot>.generate(
                  response.length, (i) => Slot(response.array[i]));
            } finally {
              lib.Utility_freeSatscardListSlotsParams(response);
            }
          });
        });
      } finally {
        freeCString(nativeSpendCode);
      }
    });
  }

  Future<Slot> satscardNew(
      Transport nfc, String spend, String chain, int handle) {
    return Future.sync(() async {
      final nativeSpendCode = allocNativeSpendCode(spend);
      final nativeChainCode = allocNativeChainCode(chain);
      try {
        return await _performAsyncCardOperation(handle, CardType.satscard,
            (lib) {
          ensure(lib.Satscard_beginNew(nativeChainCode, nativeSpendCode));
          return processTransportRequests(nfc).then((_) {
            var response = lib.Satscard_getNewResponse(handle);
            try {
              ensureStatus(response.status);
              return Slot(response.params);
            } finally {
              lib.Utility_freeSatscardSlotResponse(response);
            }
          });
        });
      } finally {
        freeCString(nativeSpendCode);
        freeCString(nativeChainCode);
      }
    });
  }

  Future<Slot> satscardUnseal(Transport nfc, String spend, int handle) {
    return Future.sync(() async {
      final nativeSpendCode = allocNativeSpendCode(spend);
      try {
        return await _performAsyncCardOperation(handle, CardType.satscard,
            (lib) {
          ensure(lib.Satscard_beginUnseal(nativeSpendCode));
          return processTransportRequests(nfc).then((_) {
            var response = lib.Satscard_getUnsealResponse(handle);
            try {
              ensureStatus(response.status);
              return Slot(response.params);
            } finally {
              lib.Utility_freeSatscardSlotResponse(response);
            }
          });
        });
      } finally {
        freeCString(nativeSpendCode);
      }
    });
  }

  Future<String> slotToWif(int satscard, int slot) {
    return performNativeOperation((lib) async {
      var response = lib.Satscard_slotToWif(satscard, slot);
      try {
        ensureStatus(response.status);
        return dartStringFromCString(response.wif);
      } finally {
        lib.Utility_freeSlotToWifResponse(response);
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
    return _cleanupFuture =
        cancelNativeOperation().whenComplete(() => _cleanupFuture = null);
  }

  Future<T> _performAsyncCardOperation<T>(
      int handle, CardType type, Future<T> Function(NativeBindings) action) {
    return performNativeOperation((lib) {
      prepareNativeThread();
      prepareForCardOperation(handle, type);
      return action(lib);
    });
  }

  /// A naive approach to ensuring concurrent operations are not being
  /// performed. This normally wouldn't be necessary but because we have a
  /// native background thread we can only guarantee the library is free of race
  /// conditions by flagging when an async operation is in progress
  Future<T> performNativeOperation<T>(
      Future<T> Function(NativeBindings) action) {
    return Future.sync(() async {
      if (_isPerformingNativeAction) {
        throw ProtocolConcurrencyError(
            "Attempt to perform a concurrent native action in CKTap");
      }

      // Cleanup is a special case where we can just safely wait for cleanup if
      // the user of the library tries performing a second action too quickly
      _awaitCleanup();
      try {
        _isPerformingNativeAction = true;
        return await action(bindings);
      } catch (e) {
        _cancelOperation();
        rethrow;
      } finally {
        _isPerformingNativeAction = false;
      }
    });
  }
}
