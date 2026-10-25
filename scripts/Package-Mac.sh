#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
if [[ "$(uname -s)" != Darwin ]]; then echo "Packaging requires macOS."; exit 1; fi
BUILD="$ROOT/build-macos"
PRODUCTS="$BUILD/CompGod_artefacts/Release"
DEST="$ROOT/dist/CompGod-1.0.0-macOS-Universal"
mkdir -p "$DEST"
for relative in VST3/CompGod.vst3 AU/CompGod.component Standalone/CompGod.app; do
    source="$PRODUCTS/$relative"
    if [[ ! -d "$source" ]]; then echo "Missing build output: $source"; exit 1; fi
    name="${relative##*/}"
    executable="$source/Contents/MacOS/CompGod"
    # Refuse to label a single-architecture artifact as Universal.
    lipo -verify_arch arm64 x86_64 "$executable"
    ditto "$source" "$DEST/$name"
    # Ad-hoc signing supports local use without an Apple Developer account.
    # No quarantine attributes are removed and no system security settings change.
    codesign --force --sign - --timestamp=none "$DEST/$name"
    codesign --verify --strict --verbose=2 "$DEST/$name"
done
cp "$ROOT/INSTALL-MAC.txt" "$DEST/INSTALL-MAC.txt"
cp "$ROOT/README.md" "$DEST/README.md"
cp "$ROOT/scripts/Install-Mac.command" "$DEST/Install-Mac.command"
chmod +x "$DEST/Install-Mac.command"
LICENSE="$ROOT/Dependencies/JUCE/LICENSE.md"
if [[ ! -f "$LICENSE" ]]; then
    JUCE_SOURCE="$(sed -n 's/^JUCE_SOURCE_DIR:STATIC=//p' "$BUILD/CMakeCache.txt" | head -n 1)"
    LICENSE="$JUCE_SOURCE/LICENSE.md"
fi
if [[ ! -f "$LICENSE" ]]; then echo "JUCE license notice is missing."; exit 1; fi
cp "$LICENSE" "$DEST/JUCE-LICENSE.md"
if [[ -f "$BUILD/CompGod-Mac.png" ]]; then cp "$BUILD/CompGod-Mac.png" "$DEST/CompGod-Mac.png"; fi
ditto -c -k --sequesterRsrc --keepParent "$DEST" "$ROOT/dist/CompGod-1.0.0-macOS-Universal.zip"
echo "Created $ROOT/dist/CompGod-1.0.0-macOS-Universal.zip"
