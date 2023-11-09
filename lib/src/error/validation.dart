import 'package:cktap_protocol/exceptions.dart';
import 'package:cktap_protocol/src/error/types.dart';
import 'package:cktap_protocol/src/native/bindings.dart';
import 'package:cktap_protocol/src/native/library.dart';

void ensureNativeThreadState(int expectedState) {
  final threadState = nativeLibrary.Core_getThreadState();
  if (threadState != expectedState) {
    throw InvalidThreadStateError(expectedState, threadState);
  }
}

void ensureNativeThreadStates(Iterable<int> allowedStates) {
  assert(allowedStates.isNotEmpty);
  final threadState = nativeLibrary.Core_getThreadState();

  int lastState = 0;
  for (final allowedState in allowedStates) {
    lastState = allowedState;
    if (threadState == allowedState) {
      return;
    }
  }

  throw InvalidThreadStateError(lastState, threadState);
}

void ensureSuccessful(int interfaceErrorCode) {
  switch (interfaceErrorCode) {
    case CKTapInterfaceErrorCode.success:
      return;
    case CKTapInterfaceErrorCode.caughtTapProtocolException:
      throw TapProtoException.fromNative();

    default:
      throw TapInterfaceError.fromCode(interfaceErrorCode);
  }
}
