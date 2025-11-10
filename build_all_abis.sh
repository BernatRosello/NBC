#!/usr/bin/env bash
set -euo pipefail

# -----------------------------------------------------------------------------
# Build all Android ABI variants of nc_unity and collect .so outputs
# -----------------------------------------------------------------------------

TARGET="//connections/unity:nc_unity"
OUT="unity_plugins"

# List of .bazelrc configs (must match the build:<name> labels in .bazelrc)
ABIS=("arm64-v8a" "x86_64" "armeabi-v7a" "x86")

echo "🚀 Building all Android ABIs for target: $TARGET"
echo "Output root: $OUT"
echo

# Ensure output root is clean
rm -rf "$OUT"
mkdir -p "$OUT"

for abi in "${ABIS[@]}"; do
    echo "=========================================================="
    echo "🏗️  Building for ABI: $abi"
    echo "=========================================================="

    # Run the Bazel build with the appropriate config
    bazel build "$TARGET" --config="$abi"

    # Locate the generated .so (and .a just in case)
    SO_FILE=$(find bazel-bin/connections/unity -maxdepth 1 -name "libnc_unity.so" 2>/dev/null || true)
    A_FILE=$(find bazel-bin/connections/unity -maxdepth 1 -name "libnc_unity.a" 2>/dev/null || true)

    ABI_OUT="$OUT/$abi"
    mkdir -p "$ABI_OUT"

    if [[ -n "$SO_FILE" ]]; then
        cp "$SO_FILE" "$ABI_OUT/"
        echo "✅ Copied $SO_FILE → $ABI_OUT/"
    else
        echo "⚠️  No .so file found for $abi"
    fi

    if [[ -n "$A_FILE" ]]; then
        cp "$A_FILE" "$ABI_OUT/"
        echo "✅ Copied $A_FILE → $ABI_OUT/"
    else
        echo "⚠️  No .a file found for $abi"
    fi

    echo
done

echo "🎉 Done!"
echo "Artifacts collected in: $OUT"
tree "$OUT" || ls -R "$OUT"
