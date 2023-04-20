import 'dart:async';
import 'dart:ffi';
import 'dart:io';
import 'dart:isolate';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';
import 'package:nfc_manager/nfc_manager.dart';
import 'package:nfc_manager/platform_tags.dart';

import 'cktap_protocol_bindings_generated.dart';

Future<int> createCKTapCard(NfcTag tag) async {
  var isoDep = IsoDep.from(tag);
  if (isoDep == null) {
    return Future.error(-1);
  }

  var initResponse = _bindings.CKTapCard_BeginInitialization();
  if (initResponse != CKTapInterfaceErrorCode.Success) {
    return Future.error(initResponse);
  }

  int threadState;
  do {
    threadState = _bindings.CKTapCard_GetThreadState();
    switch (threadState) {
      case CKTapThreadState.AwaitingTransportRequest:
      case CKTapThreadState.TransportResponseReady:
      case CKTapThreadState.ProcessingTransportResponse:
        sleep(const Duration(microseconds: 100));
        //await Future.delayed(const Duration(microseconds: 100));
        break;
      case CKTapThreadState.TransportRequestReady:
        var pointer = _bindings.CKTapCard_GetTransportRequestPointer();
        var length = _bindings.CKTapCard_GetTransportRequestLength();
        if (pointer == null) {
          return Future.error(-2);
        }
        if (length == 0) {
          return Future.error(-3);
        }

        var transportRequest = pointer.asTypedList(length);
        await isoDep.transceive(data: transportRequest).then((value) { 
          Pointer<Uint8> allocation = _bindings.CKTapCard_AllocateTransportResponseBuffer(value.length);
          if (allocation.address == 0) {
            return Future.error(-4);
          }

          var transportResponse = allocation.asTypedList(value.length);
          transportResponse.setAll(0, value);

          var errorCode = _bindings.CKTapCard_FinalizeTransportResponse();
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
  } while (threadState != CKTapThreadState.Finished && threadState != CKTapThreadState.Timeout);


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
}(););
