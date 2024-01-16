#!/bin/bash

set -e

# Don't build for the iOS simulator
echo "PLATFORM_NAME=$PLATFORM_NAME"
echo "ARCHES=$ARCHES"
echo "CONFIGURATION=$CONFIGURATION"
if [[ "$PLATFORM_NAME" == *simulator* ]]; then
  echo "tap-protocol build skipped for platform '$PLATFORM_NAME'"
  exit 0
fi

scriptDir=$(dirname "$0")
libDir="$scriptDir/Libraries"
pushd "$(realpath "$scriptDir/..")" || exit

# Dart and Flutter don't support submodules for git dependencies
echo "Updating submodules"
git submodule update --init --recursive

pushd contrib/tap-protocol || exit
echo "Building for iOS"
tools/build_ios.sh

echo "Copying build output"
mkdir -p "$libDir"
cp -fv "build/libtap-protocol.dylib" "$libDir/"

popd || exit # tap-protocol root

popd || exit # Plugin root