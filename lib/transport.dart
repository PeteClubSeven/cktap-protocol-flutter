import 'package:flutter/services.dart';

export 'package:cktap_protocol/src/nfc_manager_transport.dart';

/// Used for iOS instead of raw bytes
class Iso7816Response {
  final Uint8List data;
  final int sw1;
  final int sw2;

  const Iso7816Response(this.data, this.sw1, this.sw2);
}

/// The base class for which all NFC communication must be handled by. A
/// functionally complete subclass using the nfc_manager package has also been
/// implemented for Android & iOS, see [TransportNfcManager]. If you have other
/// needs you are free to implement your own
abstract class Transport {
  /// Should use IsoDep for Android
  Future<Uint8List> sendBytes(final Uint8List bytes);
  /// Specifically for iOS
  Future<Iso7816Response> sendIso7816(final Uint8List bytes);
}
