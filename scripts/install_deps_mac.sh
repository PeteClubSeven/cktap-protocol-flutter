#!/bin/bash

pushd "$(dirname "$0")/.." || exit

# tap-protocol dependencies
brew install autoconf automake jq libtool

# Setup SDKs
export DEPS="$(pwd)/.deps"
mkdir -p "$DEPS"
  pushd "$DEPS" || exit
    curl -o android.zip https://dl.google.com/android/repository/commandlinetools-mac-10406996_latest.zip
    unzip -n android.zip -d android

    pushd android/cmdline-tools || exit
      mkdir latest
      mv -fv bin lib *.* latest/
      echo y | latest/bin/sdkmanager "build-tools;33.0.2"
      echo y | latest/bin/sdkmanager "cmake;3.18.1"
      echo y | latest/bin/sdkmanager "ndk;21.4.7075529"
      echo y | latest/bin/sdkmanager "platform-tools"
      echo y | latest/bin/sdkmanager "platforms;android-33"
      echo y | latest/bin/sdkmanager "sources;android-33"
    popd || exit # android/cmdline-tools

    export ANDROID_HOME="$(pwd)/android"
    export ANDROID_SDK_ROOT="$ANDROID_HOME"
    export ANDROID_NDK_HOME="$ANDROID_HOME/ndk"

    if [ "$(uname -m)" == "arm64" ]; then
      echo "Downloading Flutter for ARM64"
      curl -o flutter.zip https://storage.googleapis.com/flutter_infra_release/releases/stable/macos/flutter_macos_arm64_3.7.12-stable.zip
    else
      echo "Downloading Flutter for x86_64"
      curl -o flutter.zip https://storage.googleapis.com/flutter_infra_release/releases/stable/macos/flutter_macos_3.7.12-stable.zip
    fi
    unzip -n flutter.zip
    export PATH="$(pwd)/flutter/bin:$PATH"
    flutter config --no-analytics
    flutter doctor
  popd || exit # $DEPS
popd || exit # Project directory

# Set environmental variables for Github
if [ -n "$GITHUB_ENV" ]; then
  {
    echo "ANDROID_HOME=$ANDROID_HOME"
    echo "ANDROID_SDK_ROOT=$ANDROID_SDK_ROOT"
    echo "ANDROID_NDK_HOME=$ANDROID_NDK_HOME"
    echo "PATH=$PATH"
  } >> "$GITHUB_ENV"
fi
