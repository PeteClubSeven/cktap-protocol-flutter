#!/bin/bash

scriptDir=`dirname "$0"`
echo "$0"
echo "$1"
echo "$2"

pushd "$scriptDir/../" || exit

# Dart and Flutter don't support submodules for git dependencies
git submodule update --init --recursive

pushd "$scriptDir/../contrib/tap-protocol" || exit
echo "ANDROID_NDK=\"$1\" ARCHS=\"$2\" \"$(pwd)/tools/build_android.sh\"" > "$scriptDir/last_run_command_$2.log"
ANDROID_NDK="$1" ARCHS="$2" "$(pwd)/tools/build_android.sh"

#echo 
#ARCHS=armeabi-v7a ANDROID_NDK="$1" tools/build_android.sh 
#echo 
#ARCHS=arm64-v8a ANDROID_NDK="$1" tools/build_android.sh
#echo 
#ARCHS=arm6sdsd4-v8a ANDROID_NDK="$1" tools/build_android.sh
#echo 
#ARCHS=all ANDROID_NDK="$1" tools/build_android.sh
#echo 
#ARCHS=x86_64 ANDROID_NDK="$1" tools/build_android.sh
#echo 
#ARCHS=x86 ANDROID_NDK="$1" tools/build_android.sh
#echo 
#ARCHS=arm6d4-v8a ANDROID_NDK="$1" tools/build_android.sh

popd || exit # tap-protocol

popd || exit
