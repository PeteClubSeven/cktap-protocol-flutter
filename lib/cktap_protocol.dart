import 'dart:async';
import 'dart:ffi';
import 'dart:io';
import 'dart:isolate';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';
import 'package:nfc_manager/nfc_manager.dart';
import 'package:nfc_manager/platform_tags.dart';

import 'cktap_protocol_bindings_generated.dart';

int transceiveIsoDep(int requestId, Pointer<Uint8> rawData, int dataLength) {
  var isoDep = IsoDep.from(_cktapCardTags[requestId]!);
  if (isoDep == null)
  {
    return -1;
  }

  if (isoDep.maxTransceiveLength != 0 && isoDep.maxTransceiveLength < dataLength)
  {
    return -2;
  }

  Uint8List dataToTransmit = rawData.asTypedList(dataLength);
  isoDep.transceive(data: dataToTransmit).then((value) {
    Pointer<Uint8> allocation = allocateResponse(value.lengthInBytes);
    if (allocation.address == 0)
    {
      // something went real wrong
      return;
    }

    Uint8List list =  allocation.asTypedList(value.length);
    list.setAll(0, value);

    finalizeResponse();
  }).catchError((err) {
    print('Error: $err'); // Prints 401.
  }, test: (error) {
    return error is int && error >= 400;
  });

  return 0;
}

//int createCKTapCard(TransmitDataFunction function) => _bindings.cktapcard_constructor(function);
Pointer<Uint8> allocateResponse(int sizeInBytes) => _bindings.cktapcard_allocateResponse(sizeInBytes);
int finalizeResponse() => _bindings.cktapcard_finalizeResponse();

Future<int> createCKTapCard(NfcTag tag) async {
  var isoDep = NfcA.from(tag);
  if (isoDep != null)
  {
    Uint8List handshake = Uint8List(20);
    handshake[0] = 0;
    handshake[1] = 164;
    handshake[2] = 4;
    handshake[3] = 0;
    handshake[4] = 15;
    handshake[5] = 240;
    handshake[6] = 67;
    handshake[7] = 111;
    handshake[8] = 105;
    handshake[9] = 110;
    handshake[10] = 107;
    handshake[11] = 105;
    handshake[12] = 116;
    handshake[13] = 101;
    handshake[14] = 67;
    handshake[15] = 65;
    handshake[16] = 82;
    handshake[17] = 68; 
    handshake[18] = 118; 
    handshake[19] = 49;

    return isoDep.transceive(data: handshake).then((value) { 
      print(value); 
      return 1;
    }).catchError((err) {
      print('Error: $err'); // Prints 401.
      return -1;
    }, test: (error) {
      print(error);
      return error is int && error >= 400;
    });

    final int requestID = _nextCKTapCardRequestId++;
    _cktapCardTags[requestID] = tag;
    return Future(() => _bindings.cktapcard_constructor(requestID, Pointer.fromFunction(transceiveIsoDep, 1)));

    // This doesn't work:
    final SendPort helperIsolateSendPort = await _cktapcardIsolateSendPort;
    final int requestId = _nextCKTapCardRequestId++;
    final _CKTapCardRequest request = _CKTapCardRequest(requestId, tag, 1);
    final Completer<int> completer = Completer<int>();
    _cktapCardRequests[requestId] = _CKTapCardRequestCompleterPair(request, completer);
    helperIsolateSendPort.send(request);
    return completer.future;
  }

  return Future.error(1);
}
class _CKTapCardRequest {
  final int id;
  final NfcTag tag;
  final int request;

  const _CKTapCardRequest(this.id, this.tag, this.request);
}

class _CKTapCardResponse {
  final int id;
  final int result;

  const _CKTapCardResponse(this.id, this.result);
}

/// Counter to identify [_CKTapCardRequest]s and [_CKTapCardResponse]s.
int _nextCKTapCardRequestId = 0;

class _CKTapCardRequestCompleterPair
{
  final _CKTapCardRequest request;
  final Completer<int> completer;

  const _CKTapCardRequestCompleterPair(this.request, this.completer);
}

/// Mapping from [_CKTapCardRequest] `id`s to the completers corresponding to the correct future of the pending request.
final Map<int, _CKTapCardRequestCompleterPair> _cktapCardRequests = <int, _CKTapCardRequestCompleterPair>{};
final Map<int, NfcTag> _cktapCardTags = <int, NfcTag>{};

Future<SendPort> _cktapcardIsolateSendPort = () async {
  // The helper isolate is going to send us back a SendPort, which we want to
  // wait for.
  final Completer<SendPort> completer = Completer<SendPort>();

  // Receive port on the main isolate to receive messages from the helper.
  // We receive two types of messages:
  // 1. A port to send messages on.
  // 2. Responses to requests we sent.
  final ReceivePort receivePort = ReceivePort()
    ..listen((dynamic data) {
      if (data is SendPort) {
        // The helper isolate sent us the port on which we can sent it requests.
        completer.complete(data);
        return;
      }
      if (data is _CKTapCardResponse) {
        // The helper isolate sent us a response to a request we sent.
        final Completer<int> completer = _cktapCardRequests[data.id]!.completer;
        _cktapCardRequests.remove(data.id);
        completer.complete(data.result);
        return;
      }
      throw UnsupportedError('Unsupported message type: ${data.runtimeType}');
    });

  // Start the helper isolate.
  await Isolate.spawn((SendPort sendPort) async {
    final ReceivePort helperReceivePort = ReceivePort()
      ..listen((dynamic data) {
        // On the helper isolate listen to requests and respond to them.
        if (data is _CKTapCardRequest) {
          final IsoDep? isoDep = IsoDep.from(data.tag);
          if (isoDep != null) {
            _cktapCardTags[data.id] = data.tag;
            final int result = _bindings.cktapcard_constructor(data.id, Pointer.fromFunction(transceiveIsoDep, 1));
            final _CKTapCardResponse response = _CKTapCardResponse(data.id, result);
            sendPort.send(response);
            return;
          }
        }
        throw UnsupportedError('Unsupported message type: ${data.runtimeType}');
      });

    // Send the the port to the main isolate on which we can receive requests.
    sendPort.send(helperReceivePort.sendPort);
  }, receivePort.sendPort);

  // Wait until the helper isolate has sent us back the SendPort on which we
  // can start sending requests.
  return completer.future;
}();





















/// A very short-lived native function.
///
/// For very short-lived functions, it is fine to call them on the main isolate.
/// They will block the Dart execution while running the native function, so
/// only do this for native functions which are guaranteed to be short-lived.
int sum(int a, int b) => _bindings.sum(a, b);

/// A longer lived native function, which occupies the thread calling it.
///
/// Do not call these kind of native functions in the main isolate. They will
/// block Dart execution. This will cause dropped frames in Flutter applications.
/// Instead, call these native functions on a separate isolate.
///
/// Modify this to suit your own use case. Example use cases:
///
/// 1. Reuse a single isolate for various different kinds of requests.
/// 2. Use multiple helper isolates for parallel execution.
Future<int> sumAsync(int a, int b) async {
  final SendPort helperIsolateSendPort = await _helperIsolateSendPort;
  final int requestId = _nextSumRequestId++;
  final _SumRequest request = _SumRequest(requestId, a, b);
  final Completer<int> completer = Completer<int>();
  _sumRequests[requestId] = completer;
  helperIsolateSendPort.send(request);
  return completer.future;
}

const String _libName = 'cktap_protocol';

/// The dynamic library in which the symbols for [CktapProtocolBindings] can be found.
final DynamicLibrary _dylib = () {
  if (Platform.isMacOS || Platform.isIOS) {
    return DynamicLibrary.open('$_libName.framework/$_libName');
  }
  if (Platform.isAndroid || Platform.isLinux) {
    var _ = DynamicLibrary.open('libtap-protocol.so');
    return DynamicLibrary.open('lib$_libName.so');
  }
  if (Platform.isWindows) {
    return DynamicLibrary.open('$_libName.dll');
  }
  throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
}();

/// The bindings to the native functions in [_dylib].
final CktapProtocolBindings _bindings = CktapProtocolBindings(_dylib);


/// A request to compute `sum`.
///
/// Typically sent from one isolate to another.
class _SumRequest {
  final int id;
  final int a;
  final int b;

  const _SumRequest(this.id, this.a, this.b);
}

/// A response with the result of `sum`.
///
/// Typically sent from one isolate to another.
class _SumResponse {
  final int id;
  final int result;

  const _SumResponse(this.id, this.result);
}

/// Counter to identify [_SumRequest]s and [_SumResponse]s.
int _nextSumRequestId = 0;

/// Mapping from [_SumRequest] `id`s to the completers corresponding to the correct future of the pending request.
final Map<int, Completer<int>> _sumRequests = <int, Completer<int>>{};

/// The SendPort belonging to the helper isolate.
Future<SendPort> _helperIsolateSendPort = () async {
  // The helper isolate is going to send us back a SendPort, which we want to
  // wait for.
  final Completer<SendPort> completer = Completer<SendPort>();

  // Receive port on the main isolate to receive messages from the helper.
  // We receive two types of messages:
  // 1. A port to send messages on.
  // 2. Responses to requests we sent.
  final ReceivePort receivePort = ReceivePort()
    ..listen((dynamic data) {
      if (data is SendPort) {
        // The helper isolate sent us the port on which we can sent it requests.
        completer.complete(data);
        return;
      }
      if (data is _SumResponse) {
        // The helper isolate sent us a response to a request we sent.
        final Completer<int> completer = _sumRequests[data.id]!;
        _sumRequests.remove(data.id);
        completer.complete(data.result);
        return;
      }
      throw UnsupportedError('Unsupported message type: ${data.runtimeType}');
    });

  // Start the helper isolate.
  await Isolate.spawn((SendPort sendPort) async {
    final ReceivePort helperReceivePort = ReceivePort()
      ..listen((dynamic data) {
        // On the helper isolate listen to requests and respond to them.
        if (data is _SumRequest) {
          final int result = _bindings.sum_long_running(data.a, data.b);
          final _SumResponse response = _SumResponse(data.id, result);
          sendPort.send(response);
          return;
        }
        throw UnsupportedError('Unsupported message type: ${data.runtimeType}');
      });

    // Send the the port to the main isolate on which we can receive requests.
    sendPort.send(helperReceivePort.sendPort);
  }, receivePort.sendPort);

  // Wait until the helper isolate has sent us back the SendPort on which we
  // can start sending requests.
  return completer.future;
}();
