import 'dart:ffi';
import 'dart:typed_data';

import 'native_library.dart';

/// Converts the native transport request to a Dart readable format.
/// Should only be called when there is a transport request ready.
Uint8List getNativeTransportRequest() {
  assert(nativeLibrary.Core_GetThreadState() ==
      CKTapThreadState.TransportRequestReady);

  Pointer<Uint8> requestPointer =
      nativeLibrary.Core_GetTransportRequestPointer();
  int requestLength = nativeLibrary.Core_GetTransportRequestLength();
  assert(requestPointer.address != 0);
  assert(requestLength != 0);

  return requestPointer.asTypedList(requestLength);
}

/// Passes a Dart-typed response from an NFC device to the native thread.
/// Should only be called when there is a transport request ready.
int setNativeTransportResponse(Uint8List response) {
  assert(response.isNotEmpty);
  assert(nativeLibrary.Core_GetThreadState() ==
      CKTapThreadState.TransportRequestReady);

  Pointer<Uint8> allocation =
      nativeLibrary.Core_AllocateTransportResponseBuffer(response.length);
  assert(allocation.address != 0);

  var nativeResponse = allocation.asTypedList(response.length);
  nativeResponse.setAll(0, response);

  var errorCode = nativeLibrary.Core_FinalizeTransportResponse();
  assert(errorCode == CKTapInterfaceErrorCode.Success);

  return errorCode;
}
