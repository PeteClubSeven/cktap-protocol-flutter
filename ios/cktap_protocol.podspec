#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html.
# Run `pod lib lint cktap_protocol.podspec` to validate before publishing.
#
Pod::Spec.new do |s|
  s.name             = 'cktap_protocol'
  s.version          = '0.0.1'
  s.summary          = 'A new Flutter FFI plugin project.'
  s.description      = <<-DESC
A new Flutter FFI plugin project.
                       DESC
  s.homepage         = 'http://example.com'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'Your Company' => 'email@example.com' }

  # This will ensure the source files in Classes/ are included in the native
  # builds of apps using this FFI plugin. Podspec does not support relative
  # paths, so Classes contains a forwarder C file that relatively imports
  # `../src/*` so that the C sources can be shared among all target platforms.
  s.dependency          'Flutter'
  s.platform            = :ios, '12.0'
  s.vendored_library    = 'Libraries/libtap-protocol.dylib'
  s.libraries           = 'tap-protocol'
  s.source              = { :path => '.' }
  s.source_files        = 'Classes/**/*'
  s.script_phase        = {
    :execution_position => :before_compile,
    :name => 'Build tap-protocol',
    :script => '${PODS_TARGET_SRCROOT}/build_tap_protocol.sh',
    :output_files => [
      '${PODS_TARGET_SRCROOT}/Libraries/libtap-protocol.dylib',
      '${PODS_TARGET_SRCROOT}/../contrib/tap-protocol/build/libtap-protocol.dylib',
      '${PODS_TARGET_SRCROOT}/../contrib/tap-protocol/build/contrib/bitcoin-core/libbitcoin-core.a',
      '${PODS_TARGET_SRCROOT}/../contrib/tap-protocol/contrib/bitcoin-core/src/secp256k1/build/iphoneos/libsecp256k1.a',
    ],
  }

  s.pod_target_xcconfig = {
    'CLANG_CXX_LANGUAGE_STANDARD' => 'c++17',
    'DEFINES_MODULE' => 'YES',
    'EXCLUDED_ARCHS[sdk=iphonesimulator*]' => 'i386', # Flutter.framework does not contain a i386 slice.
    'GCC_PREPROCESSOR_DEFINITIONS' => 'CKTAP_PLATFORM_IOS=1',
    'GCC_PREPROCESSOR_DEFINITIONS[sdk=iphonesimulator*]' => 'CKTAP_PLATFORM_IOS_SIMULATOR=1',
    'HEADER_SEARCH_PATHS' => [
      '${PODS_TARGET_SRCROOT}/../src/cpp',
      '${PODS_TARGET_SRCROOT}/../contrib/tap-protocol/contrib/include',
      '${PODS_TARGET_SRCROOT}/../contrib/tap-protocol/include',
      ],
    }
  s.swift_version = '5.0'
end
