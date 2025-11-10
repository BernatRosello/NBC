#!/usr/bin/env bash
set -euo pipefail

# Location where your Bazel-built .so files already end up:
OUTPUT_ROOT="unity_plugins"

# Ensure NDK is configured
if [[ -z "${ANDROID_NDK_HOME:-}" ]]; then
    echo "ERROR: ANDROID_NDK_HOME is not set."
    exit 1
fi

echo "Using NDK: $ANDROID_NDK_HOME"
echo "Copying libc++_shared.so into $OUTPUT_ROOT/<ABI>/"
echo

# ABI → LLVM Triple mapping
declare -A ABI_TRIPLES=(
  ["arm64-v8a"]="aarch64-linux-android"
  ["armeabi-v7a"]="arm-linux-androideabi"
  ["x86_64"]="x86_64-linux-android"
  ["x86"]="i686-linux-android"
)

for ABI in "${!ABI_TRIPLES[@]}"; do
    TRIPLE="${ABI_TRIPLES[$ABI]}"
    
    SRC="$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/$TRIPLE/libc++_shared.so"
    DST_DIR="$OUTPUT_ROOT/$ABI"
    DST="$DST_DIR/libc++_shared.so"

    if [[ ! -f "$SRC" ]]; then
        echo "WARNING: libc++_shared.so not found for ABI $ABI at:"
        echo "  $SRC"
        continue
    fi

    mkdir -p "$DST_DIR"
    cp "$SRC" "$DST"

    echo "✅ Copied $ABI → $DST"
done

echo
echo "Done."
