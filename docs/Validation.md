# CompGod 1.0.0 validation

Validated locally on 25 October 2026, Windows x64. Release build with Visual Studio 2026 / MSVC 19.51, Windows SDK 10.0.26100.0, CMake 4.3.1, JUCE 8.0.6.

## Results

- Release VST3 bundle and standalone application compile successfully.
- DSP test suite: 16 assertions pass, covering the static compression curve, soft-knee bypass region, steady gain, bypass transition, dry mix, stereo balance, fixed and manual recovery, combined optical memory, five sample rates (22.05-192 kHz), extreme settings, invalid input sanitization and silence.
- Plugin/host test suite: 18 assertions pass without rendering, covering edited session recall, malformed state, real audio processing, dry mix, mono external sidechain, all six factory programs, VST3 discovery, native VST3 instantiation, hosted processing, hosted state and hosted editor creation.
- Render validation adds five passing checks for a native knob updating its host parameter, time-unit parsing and display, and full/compact editor screenshots.
- The actual native editor was rendered at 1100 x 760 and 880 x 608. Both were inspected for clipping, overlap, alignment, readable values and control visibility. The reduction trace comes from synthetic audio processed by the compressor while the editor's native timers run.

Raw assertion output and generated UI previews remain in the local workspace and are excluded from the source repository. CTest registers two suites: `dsp` and `plugin`.

## Scope

VST3 validation uses JUCE's native VST3 host implementation to load the compiled bundle from disk. This confirms an actual loadable plugin, rather than only exercising shared source code. It is not a test in FL Studio, Ableton, Cubase, or another commercial DAW. No listening comparison or measured comparison against a physical CL 1B was performed. Windows x64 binaries are delivered. A Mac source/build kit is also supplied; native macOS compilation, DAW-specific setup, Developer ID signing, notarization, and independent Steinberg validator certification have not been performed here.

The reference PDF and its extracted text remain local and are excluded from this repository.

## macOS port

The build now adds Audio Unit on Apple platforms and supports `arm64;x86_64` universal outputs. Tests obtain the VST3 bundle location from the actual CMake target, rather than a Windows/Release-specific relative path. A JUCE timer/event loop replaces Windows-only message APIs in screenshot tests. Mac UI text uses the platform's default sans-serif font; standalone microphone permissions are declared.

The port is regression-tested on Windows with both CTest suites and the screenshot/interaction checks. Mac shell scripts are syntax-checked with Bash. The build kit includes pinned JUCE 8.0.6 source for offline configuration. These checks do not substitute for running the Mac compiler or loading the plugin on Mac.

`Build-Mac.command` runs DSP and VST3 host tests on the Mac, renders UI previews, and refuses to create a Universal package unless `lipo` confirms arm64 and x86_64 in all three bundles. The installer verifies signatures and runs `auval -v aufx Cg01 CgAu`. The optional CI workflow repeats the build and Audio Unit validation on Apple Silicon and Intel runners. Those Mac-specific results remain pending until a Mac runs the supplied scripts.
