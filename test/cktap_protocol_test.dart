import 'package:flutter_test/flutter_test.dart';
import 'package:cktap_protocol/cktap_protocol.dart';
import 'package:cktap_protocol/cktap_protocol_platform_interface.dart';
import 'package:cktap_protocol/cktap_protocol_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockCKTapProtocolPlatform
    with MockPlatformInterfaceMixin
    implements CKTapProtocolPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final CKTapProtocolPlatform initialPlatform = CKTapProtocolPlatform.instance;

  test('$MethodChannelCKTapProtocol is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelCKTapProtocol>());
  });

  test('getPlatformVersion', () async {
    CKTapProtocol cktapProtocolPlugin = CKTapProtocol();
    MockCKTapProtocolPlatform fakePlatform = MockCKTapProtocolPlatform();
    CKTapProtocolPlatform.instance = fakePlatform;

    expect(await cktapProtocolPlugin.getPlatformVersion(), '42');
  });
}
