#!/bin/bash

if [ -z "$1" ] || [ -z "$2" ]; then
  echo "Usage: package-release.sh version destdir [--no-package]"
  exit 1
fi

DXUP_VERSION="$1"
DXUP_SRC_DIR=`dirname $(readlink -f $0)`
DXUP_BUILD_DIR=$(realpath "$2")"/dxvk-$DXUP_VERSION"
DXUP_ARCHIVE_PATH=$(realpath "$2")"/dxvk-$DXUP_VERSION.tar.gz"

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

  cp "$DXUP_BUILD_DIR/install.$1/bin/d3d10.dll" "$DXUP_BUILD_DIR/x$1/d3d10.dll"
  cp "$DXUP_BUILD_DIR/install.$1/bin/d3d10core.dll" "$DXUP_BUILD_DIR/x$1/d3d10core.dll"
  cp "$DXUP_BUILD_DIR/install.$1/bin/d3d10_1.dll" "$DXUP_BUILD_DIR/x$1/d3d10_1.dll"
  cp "$DXUP_BUILD_DIR/install.$1/bin/d3d10_1core.dll" "$DXUP_BUILD_DIR/x$1/d3d10_1core.dll"
  cp "$DXUP_BUILD_DIR/install.$1/bin/setup_dxvk.sh" "$DXUP_BUILD_DIR/x$1/setup_dxvk.sh"
  
  rm -R "$DXUP_BUILD_DIR/wine.$1"
  rm -R "$DXUP_BUILD_DIR/build.$1"
  rm -R "$DXUP_BUILD_DIR/install.$1"
}

function package {
  cd "$DXUP_BUILD_DIR/.."
  tar -czf "$DXUP_ARCHIVE_PATH" "dxvk-$DXUP_VERSION"
  rm -R "dxvk-$DXUP_VERSION"
}

build_arch 64
build_arch 32

if [ "$3" != "--no-package" ]; then
  package
fi