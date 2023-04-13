
import 'cktap_protocol_platform_interface.dart';

class CKTapProtocol {
  Future<String?> getPlatformVersion() {
    return CKTapProtocolPlatform.instance.getPlatformVersion();
  }
}
