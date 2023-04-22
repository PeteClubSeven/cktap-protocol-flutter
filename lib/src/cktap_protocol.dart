import 'dart:async';
import 'dart:ffi';
import 'dart:io';
import 'dart:isolate';
import 'dart:typed_data';

import 'package:cktap_protocol/src/cktapcard.dart';
import 'package:ffi/ffi.dart';
import 'package:nfc_manager/nfc_manager.dart';
import 'package:nfc_manager/platform_tags.dart';

import 'internal/generated_bindings.dart';

Future<CKTapCard> createCKTapCard(NfcTag tag,
    {bool pollAllSatscardSlots = false}) async {
  var isoDep = IsoDep.from(tag);
  if (isoDep == null) {
    return Future.error(-1);
  }

  var initResponse = _bindings.Core_BeginInitialization();
  if (initResponse != CKTapInterfaceErrorCode.Success) {
    return Future.error(initResponse);
  }

  int threadState;
  do {
    threadState = _bindings.Core_GetThreadState();
    switch (threadState) {
      case CKTapThreadState.AwaitingTransportRequest:
      case CKTapThreadState.TransportResponseReady:
      case CKTapThreadState.ProcessingTransportResponse:
        await Future.delayed(const Duration(microseconds: 100));
        break;
      case CKTapThreadState.TransportRequestReady:
        var pointer = _bindings.Core_GetTransportRequestPointer();
        var length = _bindings.Core_GetTransportRequestLength();
        if (pointer == null) {
          return Future.error(-2);
        }
        if (length == 0) {
          return Future.error(-3);
        }

        var transportRequest = pointer.asTypedList(length);
        await isoDep.transceive(data: transportRequest).then((value) {
          Pointer<Uint8> allocation =
              _bindings.Core_AllocateTransportResponseBuffer(value.length);
          if (allocation.address == 0) {
            return Future.error(-4);
          }

          var transportResponse = allocation.asTypedList(value.length);
          transportResponse.setAll(0, value);

          var errorCode = _bindings.Core_FinalizeTransportResponse();
          if (errorCode != CKTapInterfaceErrorCode.Success) {
            return Future.error(errorCode);
          }

          return 1;
        }).catchError((err) {
          print('Error: $err');
          return -5;
        }, test: (error) {
          print(error);
          return error is int && error >= 400;
        });

        break;
    }
  } while (threadState != CKTapThreadState.Finished &&
      threadState != CKTapThreadState.Timeout);

  // Finalize the initialization of the card and prepare the result
  if (threadState == CKTapThreadState.Finished) {
    var response = _bindings.Core_FinalizeRecentOperation();
    if (response.errorCode != CKTapInterfaceErrorCode.Success) {
      return Future.error(response.errorCode);
    }


  }

  return 0;
}

/// The bindings to the native functions in [CKTapProtocolBindings] and its dependencies.
final CKTapProtocolBindings _bindings = CKTapProtocolBindings(() {
  const String libName = 'cktap_protocol';

  if (Platform.isMacOS || Platform.isIOS) {
    return DynamicLibrary.open('$libName.framework/$libName');
  }
  if (Platform.isAndroid || Platform.isLinux) {
    var _ = DynamicLibrary.open('libtap-protocol.so');
    return DynamicLibrary.open('lib$libName.so');
  }
  if (Platform.isWindows) {
    return DynamicLibrary.open('$libName.dll');
  }
  throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
}());
