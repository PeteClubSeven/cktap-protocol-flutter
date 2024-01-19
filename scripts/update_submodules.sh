#!/bin/bash
set -e

scriptDir="$(dirname "$0")"
rootDir="$scriptDir/.."

if [ -d "$rootDir/.git" ] && [ "$(ls -A "$rootDir/.git")" ]; then
  pushd "$rootDir" || exit
  git submodule update --init --recursive
  popd || exit
fi
