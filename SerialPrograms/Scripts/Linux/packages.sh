#!/bin/bash
#
# Usage:
#   ./packages.sh [PACKAGE_TYPE]
#
#   PACKAGE_TYPE (optional):
#     DEB
#     RPM
#     TGZ
#     PKG
#

# Config
AVAILABLE_TYPES=("DEB" "RPM" "TGZ" "PKG")
declare -A TOOLS=(
  [DEB]="dpkg"
  [RPM]="rpmbuild"
  [TGZ]=""
  [PKG]="makepkg"
)

SCRIPT_DIR="$(realpath "$(dirname "$0")")"
SOURCE_DIR="${SCRIPT_DIR}/../../../SerialPrograms/"
BUILD_DIR="${SOURCE_DIR}/build/"


# Arg Parse
BUILD_TYPE="$1"
if [[ -n "$BUILD_TYPE" ]]; then
    BUILD_TYPE="${BUILD_TYPE^^}"
    if [[ ! " ${AVAILABLE_TYPES[*]} " =~ " $BUILD_TYPE " ]]; then
        echo "Invalid package type: $BUILD_TYPE"
        echo "Valid types: ${AVAILABLE_TYPES[*]}"
        exit 1
    fi
    TARGETS=("$BUILD_TYPE")
else
    TARGETS=("${AVAILABLE_TYPES[@]}")
fi


# Check Tools
echo "Checking required packaging tools..."
for pkg in "${TARGETS[@]}"; do
    TOOL=${TOOLS[$pkg]}
    if [[ -n "$TOOL" ]]; then
        if ! command -v "$TOOL" >/dev/null 2>&1; then
            echo "  [MISSING] $pkg requires $TOOL"
            TARGETS=("${TARGETS[@]/$pkg}")
        else
            echo "  [OK] $pkg -> $TOOL found"
        fi
    else
        echo "  [OK] $pkg -> no external tool required"
    fi
done


# Check CCache
CCACHE_ARGS=()
if command -v ccache >/dev/null 2>&1; then
  echo "  [INFO] ccache found, caching compilation"
  CCACHE_ARGS=(-DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache)
fi


# Build CMake
cd "$SOURCE_DIR"
cmake "$SOURCE_DIR" -G Ninja -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS="-O2 -g0 -fno-record-gcc-switches -fno-ident -Wno-odr" \
  -DCMAKE_CXX_FLAGS="-O2 -g0 -fno-record-gcc-switches -fno-ident -Wno-odr" \
  "${CCACHE_ARGS[@]}"

cd "$SOURCE_DIR/build"
ninja -j$(( $(nproc) - 1))


# Build Packages
cd "$BUILD_DIR"
for pkg in "${TARGETS[@]}"; do
  case "$pkg" in
    DEB|RPM|TGZ)
      echo "Building $pkg package with CPack..."
      cpack -G "$pkg"
      ;;
    PKG)
      echo "Building PKG package with makepkg..."
      cp "$SCRIPT_DIR/PKGBUILD" "."
      git -C "$SOURCE_DIR/.." archive --format=tar.gz --prefix=Arduino-Source/ \
        HEAD > "Arduino-Source.tar.gz"
      makepkg -Cf
      ;;
    *)
      echo "Unknown package type: $pkg"
      ;;
  esac
done

echo "Package creation complete."
echo "Packages can be found here: $BUILD_DIR"