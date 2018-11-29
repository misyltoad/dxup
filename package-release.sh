#!/bin/bash

if [ -z "$1" ] || [ -z "$2" ]; then
  echo "Usage: package-release.sh version destdir [--no-package]"
  exit 1
fi

DXUP_VERSION="$1"
DXUP_SRC_DIR=`dirname $(readlink -f $0)`
DXUP_BUILD_DIR=$(realpath "$2")"/dxup-$DXUP_VERSION"
DXUP_ARCHIVE_PATH=$(realpath "$2")"/dxup-$DXUP_VERSION.tar.gz"

function build_arch {
  export WINEARCH="win$1"
  export WINEPREFIX="$DXUP_BUILD_DIR/wine.$1"
  
  cd "$DXUP_SRC_DIR"

  meson --cross-file "$DXUP_SRC_DIR/build-win$1.txt"  \
        --buildtype "release"                         \
        --prefix "$DXUP_BUILD_DIR/install.$1"         \
        --unity off                                   \
        --strip                                       \
        -Denable_tests=false                          \
        "$DXUP_BUILD_DIR/build.$1"

  cd "$DXUP_BUILD_DIR/build.$1"
  ninja install

  mkdir "$DXUP_BUILD_DIR/x$1"

  cp "$DXUP_BUILD_DIR/install.$1/bin/d3d10_1.dll" "$DXUP_BUILD_DIR/x$1/d3d10_1.dll"
  cp "$DXUP_BUILD_DIR/install.$1/bin/dxgi.dll" "$DXUP_BUILD_DIR/x$1/dxgi.dll"
  cp "$DXUP_BUILD_DIR/install.$1/bin/setup_dxup.sh" "$DXUP_BUILD_DIR/x$1/setup_dxup.sh"
  

  rm -R "$DXUP_BUILD_DIR/build.$1"
  rm -R "$DXUP_BUILD_DIR/install.$1"
}

function package {
  cd "$DXUP_BUILD_DIR/.."
  tar -czf "$DXUP_ARCHIVE_PATH" "dxup-$DXUP_VERSION"
  rm -R "dxup-$DXUP_VERSION"
}

build_arch 64
build_arch 32

if [ "$3" != "--no-package" ]; then
  package
fi