import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'cktap_protocol_platform_interface.dart';

/// An implementation of [CKTapProtocolPlatform] that uses method channels.
class MethodChannelCKTapProtocol extends CKTapProtocolPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('cktap_protocol');

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }
}
