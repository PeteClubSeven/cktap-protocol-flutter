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
  s.homepage         = 'https://github.com/PeteClubSeven/cktap-protocol-flutter'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'Peter' => 'peter@talesofapleb.com' }

  # This will ensure the source files in Classes/ are included in the native
  # builds of apps using this FFI plugin. Podspec does not support relative
  # paths, so Classes contains a forwarder C file that relatively imports
  # `../src/*` so that the C sources can be shared among all target platforms.
  s.dependency          'Flutter'
  s.platform            = :ios, '12.0'
  s.source              = { :path => '.' }
  s.source_files        = 'Classes/**/*'
  s.script_phase        = {
    :execution_position => :before_compile,
    :name => 'Build tap-protocol',
    :script => '${PODS_TARGET_SRCROOT}/build_tap_protocol.sh',
    :output_files => [
      '${PODS_TARGET_SRCROOT}/Libraries/libtap-protocol.a',
      '${PODS_TARGET_SRCROOT}/Libraries/libbitcoin-core.a',
      '${PODS_TARGET_SRCROOT}/Libraries/libsecp256k1.a',
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
    ],
    'LIBRARY_SEARCH_PATHS' => '${PODS_TARGET_SRCROOT}/Libraries',
    'OTHER_LDFLAGS[sdk=iphoneos*]' => '-l"tap-protocol" -l"bitcoin-core" -l"secp256k1"',
    'STRIP_STYLE' => 'non-global',
    'SYSTEM_HEADER_SEARCH_PATHS' => [
      '${PODS_TARGET_SRCROOT}/../contrib/tap-protocol/contrib/include',
      '${PODS_TARGET_SRCROOT}/../contrib/tap-protocol/include',
    ],
  }
  s.swift_version = '5.0'
end
