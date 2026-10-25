#!/bin/bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
if [[ "$(uname -s)" != Darwin ]]; then echo "Run this installer on macOS."; exit 1; fi
for bundle in CompGod.vst3 CompGod.component CompGod.app; do
    if [[ ! -d "$ROOT/$bundle" ]]; then echo "Missing $bundle. Keep this script beside the built bundles."; exit 1; fi
    codesign --verify --strict "$ROOT/$bundle"
done
# Only this user's plugin and application folders are written. No sudo required.
mkdir -p "$HOME/Library/Audio/Plug-Ins/VST3" "$HOME/Library/Audio/Plug-Ins/Components" "$HOME/Applications"
ditto "$ROOT/CompGod.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/CompGod.vst3"
ditto "$ROOT/CompGod.component" "$HOME/Library/Audio/Plug-Ins/Components/CompGod.component"
ditto "$ROOT/CompGod.app" "$HOME/Applications/CompGod.app"
echo "Installed CompGod VST3, Audio Unit and standalone app for $(whoami)."
echo "Restart your DAW and rescan plugins."
echo "Checking the Audio Unit with Apple's validator..."
if /usr/bin/auval -v aufx Cg01 CgAu; then
    echo "Audio Unit validation passed."
else
    echo "Installed, but Audio Unit validation did not pass yet."
    echo "Close and reopen the DAW, then retry: auval -v aufx Cg01 CgAu"
    exit 1
fi
