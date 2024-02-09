# Coinkite Tap Protocol Flutter

A flutter library implementing the Coinkite tap protocol for use with the Satscard and Tapsigner. The plugin is effectively a wrapper around Nunchuk's [tap-protocol](https://github.com/nunchuk-io/tap-protocol) library.

## Current Status

The project is currently under development. It is being developed primarily to allow Satscard support in the [Breez](https://breez.technology/mobile/) app. Once that bounty is complete this plugin will be updated with full support for the Satscard and Tapsigner.

### Platform Support
- [x] Android
- [x] iOS (macOS only)

### Feature Support
- [x] Build on Windows
- [x] Build on macOS
- [x] Build on Linux
- [x] Read Satscards
- [x] Read Tapsigners
- [x] Perform every Satscard-specific NFC operation

### Todo
- [ ] Support macOS
- [ ] Support Linux
- [ ] Support Web
- [ ] Support Windows
- [ ] Perform every CKTapCard NFC operation
- [ ] Perform every Tapsigner-specific NFC operation
- [ ] Expose the tap-protocol utility functions
- [ ] Expose the alternative Tapsigner HWI API
- [ ] Allow concurrent native operations (currently errors out to avoid crashing)

## Getting Started

You must have the tools required for tap-protocol and libsecp256k1 to build, please follow these instructions:
- All platforms
  - Make sure the Android SDK is installed and $ANDROID_HOME points to a valid location with the command line tools installed.
- macOS 13 or newer
  - `brew install cmake`
- macOS 12 or older
  - `brew install cmake coreutils`

## Project Stucture

This template uses the following structure:

* `lib`: Contains the Dart code that defines the API of the plugin, and which
  calls into the native code using `dart:ffi`.

* `src/cpp`: Contains the native source code, and a CMakeLists.txt file for building
  that source code into a dynamic library.

* platform folders (`android` & `ios`): Contains the build files for building and bundling
  the native code library with the platform application.
