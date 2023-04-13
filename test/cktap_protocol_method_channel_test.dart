import 'package:flutter/services.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:cktap_protocol/cktap_protocol_method_channel.dart';

void main() {
  MethodChannelCKTapProtocol platform = MethodChannelCKTapProtocol();
  const MethodChannel channel = MethodChannel('cktap_protocol');

  TestWidgetsFlutterBinding.ensureInitialized();

  setUp(() {
    channel.setMockMethodCallHandler((MethodCall methodCall) async {
      return '42';
    });
  });

  tearDown(() {
    channel.setMockMethodCallHandler(null);
  });

  test('getPlatformVersion', () async {
    expect(await platform.getPlatformVersion(), '42');
  });
}
