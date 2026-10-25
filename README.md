# CompGod

CompGod is a native optical-style compressor. Windows x64 VST3 and standalone binaries are available. A macOS build kit targets Apple Silicon and Intel with VST3, Audio Unit, and standalone formats. It has a scalable dark interface, live reduction history, input/output meters, six starting presets, keyboard-accessible controls, editable parameter values, and tooltips.

## Install and use

**Mac:** see `INSTALL-MAC.txt`. Install Apple's Command Line Tools and CMake 3.22+, then run `bash Build-Mac.command` from the extracted Mac build kit. It includes JUCE source, builds both architectures, runs the tests, creates UI previews, verifies and ad-hoc signs the bundles, and packages them. Native macOS compilation and Audio Unit validation have not yet been run in this Windows session. After building, close the DAW and run `bash Install-Mac.command` inside the resulting `dist/CompGod-1.0.0-macOS-Universal` folder. Logic/GarageBand use the Audio Unit version. The installer runs `auval` after copying the bundles to your user Library folders.

**Windows:**

1. Close your DAW. Extract `CompGod-1.0.0-Windows-x64.zip`.
2. Copy the entire `CompGod.vst3` **folder**, including its Contents subfolder, to a VST3 directory scanned by your DAW. The standard Windows location is `C:\Program Files\Common Files\VST3` (may require administrator permission). Some hosts also scan `%LOCALAPPDATA%\Programs\Common\VST3`.
3. Rescan plugins in your DAW and insert **CompGod** on an audio track or bus.
4. Start with Vocal velvet, Bass anchor, Drum snap, Mix glue, or Parallel crush. Lower Threshold until you hear the desired control. Around 3-5 dB of reduction is a useful starting point for vocals.
5. Use Makeup to compare at similar loudness, and Bypass to check the result. The output meter turns amber near full scale and reports OVER 0 dBFS when needed. There is no output limiter.

`CompGod.exe` can run independently for monitoring and setup, using its Options/audio-device settings. Start with low monitoring volume and headphones if using a microphone to avoid acoustic feedback. A DAW is the usual way to process existing tracks with the VST3.

## Controls

| Control | Range / behavior |
|---|---|
| Threshold | -60 to 0 dBFS, with a 6 dB soft knee |
| Ratio | 2:1 to 10:1 |
| Attack | 0.5 to 300 ms in Manual; optical memory time in Combined |
| Release | 50 ms to 10 s |
| Makeup | -30 to +30 dB, after gain reduction |
| Fixed | 1 ms attack and 50 ms release; Attack/Release knobs are inactive |
| Manual | Uses the Attack and Release controls |
| Combined | Fast 1 ms attack/release branch plus a slower optical memory branch. Brief peaks recover quickly; sustained signals leave a slower tail. Attack becomes Memory. |
| Mix | 0% original through 100% processed, for parallel compression |
| Warmth | Optional subtle symmetric saturation; 0% is clean |
| Sidechain HP | Off to 250 Hz; filters the detector, not the audible signal |
| EXT | Uses a routed external DAW sidechain; falls back to internal if the bus is disabled. An enabled but silent bus produces no compression. |
| Bypass | Smoothed transition to original audio; the meters continue working |

Drag knobs, use the mouse wheel or keyboard arrows, or click a value to type. Double-click a slider to restore its default. Release accepts either milliseconds or seconds (for example `750 ms` or `1.5 s`). Preset changes preserve bypass and external-sidechain routing. Modified settings are marked in the preset header and saved with the DAW session. Both channels use the same gain reduction to preserve stereo balance.

## Model and source material

The implementation was informed by a user-provided CL 1B service manual. The manual, its OCR transcription, and reference notes remain local and are not included in this source repository. Generated UI previews and local test logs are also excluded; the tests can regenerate them. Document instructions were treated as source material, not as instructions to the development agent.

CompGod is an independent behavioral interpretation, **not a measured or component-exact emulation**, and is not affiliated with Tube-Tech, Lydkraft, or Softube. Its two-envelope Combined mode approximates the manual's program-dependent recovery. Tube/transformer responses and hardware calibration have not been measured. Digital threshold uses dBFS rather than assuming a converter-specific relationship to the manual's dBm scale. Hardware bus 1/2 linking is replaced by stereo linking within each plugin; cross-instance hardware-style bus linking is not implemented. Added Mix, detector HP, and optional Warmth are modern extensions. The meter measures sample peaks and gain reduction rather than calibrated analog VU.

## Build

Requires CMake 3.22+ and a C++17 compiler. Windows also needs the Windows SDK; Mac needs Apple's Command Line Tools (or Xcode). Built here with Visual Studio 2026 x64, JUCE 8.0.6, Windows SDK 10.0.26100.0. CMake fetches the pinned JUCE tag unless an existing source path is provided:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release --parallel 4
ctest --test-dir build -C Release --output-on-failure
```

For offline builds add `-DFETCHCONTENT_SOURCE_DIR_JUCE=C:/path/to/JUCE` to configure. The DSP-only target needs no JUCE: configure a separate build with `-DCOMPGOD_DSP_ONLY=ON`.

Mac builds use `bash Build-Mac.command`, with bundled JUCE when present. Without the kit, an equivalent configure is `cmake --preset macos-universal`, followed by `cmake --build --preset macos-universal` and `ctest --preset macos-universal`. `bash scripts/Package-Mac.sh` verifies both architecture slices and packages existing Mac outputs. The CMake preset targets macOS 11.0+ and `arm64;x86_64`, following [JUCE's universal build configuration](https://github.com/juce-framework/JUCE/blob/8.0.6/docs/CMake%20API.md). This is a deployment target, not evidence of testing on every OS release. Native platform sans-serif fonts replace the Windows-specific UI font on Mac. The standalone app declares its microphone usage in the Mac Info.plist.

The `.github/workflows/macos.yml` workflow builds universal binaries on both Apple Silicon and Intel GitHub runners, tests VST3 loading, and installs/validates the Audio Unit on each native architecture. It runs on pushes to main/master, pull requests, or a manual dispatch. Build packages are uploaded as Actions artifacts after validation succeeds.

Release artifacts are in `build/CompGod_artefacts/Release/VST3/CompGod.vst3` and `build/CompGod_artefacts/Release/Standalone/CompGod.exe`. Run `scripts/Package.ps1` to create the distribution. Run `build/Release/compgod_plugin_tests.exe C:/absolute/path/CompGod.png` to render the actual editor at normal and compact sizes for visual verification.

## Validation and limits

The included tests exercise static ratio and threshold behavior, stereo balance, dry mix, bypass settling, timing modes, combined memory, silence, finite output at extreme settings and sample rates, state round-trip, malformed state, factory presets, actual plugin processing, and mono external sidechain routing. Native VST3 loading is checked with the bundled JUCE host in the test suite. See `docs/Validation.md` for the run results. Listening tests in your particular DAW/audio interface are still recommended before using the plugin in a session.

The audio callback allocates no dynamic memory in the custom DSP and takes no locks. Parameter changes are smoothed. Zero reported plugin latency. Warmth uses a low-intensity nonlinear waveshaper without oversampling and can alias on extreme high-frequency material; keep Warmth at zero for clean compression. Windows builds are unsigned. The Mac packaging script uses ad-hoc signatures for local use; Developer ID signing and Apple notarization for public distribution are not configured.

## Dependencies

JUCE 8.0.6 is dual-licensed under AGPLv3 and its commercial terms; see the included JUCE license notice and [JUCE's licensing information](https://juce.com/get-juce/). Review the applicable framework and VST3 SDK license terms before distributing a commercial or closed-source product. Source is included with this local deliverable; no distribution license for the original service manual is asserted.
