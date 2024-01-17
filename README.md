# Coinkite Tap Protocol Flutter

A flutter library implementing the Coinkite tap protocol for use with the Satscard and Tapsigner. The plugin is effectively a wrapper around Nunchuk's [tap-protocol](https://github.com/nunchuk-io/tap-protocol) library.

## Current Status

The project is currently under development. It is being developed primarily to allow Satscard support in the [Breez](https://breez.technology/mobile/). Once that bounty is complete this plugin will be updated with full support for the Satscard and Tapsigner.

### Platform Support
- [x] Android
- [ ] iOS (in progress): https://github.com/PeteClubSeven/cktap-protocol-flutter/tree/feature/ios-support
- [ ] macOS
- [ ] Linux
- [ ] Windows

### Feature support

- [ ] Support building on Windows
- [x] Support building on macOS
- [x] Support building on Linux
- [x] Reading Satscards
- [x] Reading Tapsigners
- [ ] Performing every CKTapCard NFC operation
- [x] Performing every Satscard-specific NFC operation
- [ ] Performing every Tapsigner-specific NFC operation
- [ ] Exposing the tap-protocol utility functions
- [ ] Exposing the alternative Tapsigner HWI API
- [ ] Allow concurrent native operations (currently errors out to avoid crashing)

## Getting Started

You must have the tools required for libsecp256k1 to build. Run the following commands:
- On macOS
  - `brew install autoconf automake cmake libtool`
  - macOS v12 and lower may require `brew install coreutils`
- On Ubuntu
  -  `sudo apt install autoconf automake gcc libtool`

## Project stucture

This template uses the following structure:

* `src/cpp`: Contains the native source code, and a CMakeLists.txt file for building
  that source code into a dynamic library.

* `lib`: Contains the Dart code that defines the API of the plugin, and which
  calls into the native code using `dart:ffi`.

* platform folders (`android` & `ios`): Contains the build files for building and bundling
  the native code library with the platform application.
