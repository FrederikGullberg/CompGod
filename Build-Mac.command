#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT"
export PATH="/opt/homebrew/bin:/usr/local/bin:/Applications/CMake.app/Contents/bin:$PATH"
if [[ "$(uname -s)" != Darwin ]]; then
    echo "Run this script on your Mac. It builds Apple Silicon + Intel binaries."
    exit 1
fi
if ! xcrun --find clang++ >/dev/null 2>&1; then
    echo "Install Apple's Command Line Tools with: xcode-select --install"
    echo "Then run this script again."
    exit 1
fi
if ! command -v cmake >/dev/null 2>&1; then
    echo "Install CMake 3.22 or newer from https://cmake.org/download/"
    echo "Place CMake.app in Applications, or install it with: brew install cmake"
    exit 1
fi
args=(-S "$ROOT" -B "$ROOT/build-macos" -G "Unix Makefiles"
      -DCMAKE_BUILD_TYPE=Release '-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64'
      -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0)
if [[ -f "$ROOT/Dependencies/JUCE/CMakeLists.txt" ]]; then
    args+=("-DFETCHCONTENT_SOURCE_DIR_JUCE=$ROOT/Dependencies/JUCE")
fi
cmake "${args[@]}"
cmake --build "$ROOT/build-macos" --config Release --parallel "${COMPGOD_BUILD_JOBS:-4}"
ctest --test-dir "$ROOT/build-macos" -C Release --output-on-failure
# Produces native UI previews in addition to the normal host and DSP tests.
"$ROOT/build-macos/compgod_plugin_tests" "$ROOT/build-macos/CompGod-Mac.png"
bash "$ROOT/scripts/Package-Mac.sh"
echo
echo "Build complete. Open dist/CompGod-1.0.0-macOS-Universal."
echo "Run Install-Mac.command inside that folder to install for your user account."
