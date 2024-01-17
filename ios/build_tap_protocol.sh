#!/bin/zsh
set -e

export PATH=$PATH:/opt/homebrew/bin/

# Don't build for the iOS simulator
echo "PLATFORM_NAME=$PLATFORM_NAME"
echo "ARCHES=$ARCHES"
echo "CONFIGURATION=$CONFIGURATION"
if [[ "$PLATFORM_NAME" == *simulator* ]]; then
  echo "Skipping tap-protocol build for '$PLATFORM_NAME'"
  exit 0
fi

scriptDir=$(dirname "$0")
libDir="$scriptDir/Libraries"
pushd "$(realpath "$scriptDir/..")" || exit

# Dart and Flutter don't support submodules for git dependencies
echo "Updating submodules"
git submodule update --init --recursive

pushd contrib/tap-protocol || exit

# Skip the built in cmake generation by creating a build folder
mkdir -p "build/$CONFIGURATION"

echo "Building libsecp256k1 for iOS"
tools/build_ios.sh || true

pushd "build/$CONFIGURATION" || exit

echo "Building tap-protocol for iOS"
cmake ../../ -DCMAKE_TOOLCHAIN_FILE=ios.toolchain.cmake -DPLATFORM=OS64 -DBUILD_SHARED_LIB_TAPPROTOCOL=0
if [ -f /usr/sbin/sysctl ]; then
  numCores=$(sysctl -n hw.ncpu)
  make -j "$numCores"
else
  make
fi

echo "Copying build output to Libraries subdirectory"
mkdir -p "$libDir"
cp -fv \
  "libtap-protocol.a" \
  "contrib/bitcoin-core/libbitcoin-core.a" \
  "../../contrib/bitcoin-core/src/secp256k1/build/iphoneos/libsecp256k1.a" \
  "$libDir/"

popd || exit # build
popd || exit # tap-protocol root
popd || exit # Plugin root
