import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'cktap_protocol_method_channel.dart';

abstract class CKTapProtocolPlatform extends PlatformInterface {
  /// Constructs a CKTapProtocolPlatform.
  CKTapProtocolPlatform() : super(token: _token);

  static final Object _token = Object();

  static CKTapProtocolPlatform _instance = MethodChannelCKTapProtocol();

  /// The default instance of [CKTapProtocolPlatform] to use.
  ///
  /// Defaults to [MethodChannelCKTapProtocol].
  static CKTapProtocolPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [CKTapProtocolPlatform] when
  /// they register themselves.
  static set instance(CKTapProtocolPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}
