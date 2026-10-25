#include "Compressor.h"
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
using namespace compgod;
int failures = 0;
void expect(bool condition, const char* message) {
    std::cout << (condition ? "PASS " : "FAIL ") << message << '\n';
    if (!condition) ++failures;
}
float settle(Compressor& c, float level, int samples = 96000) {
    float out = 0;
    for (int i = 0; i < samples; ++i) { float l = level, r = level; c.process(l, r, level, level); out = l; }
    return out;
}
int main() {
    expect(std::abs(Compressor::reduction(-12, -24, 4) - 9) < .001f, "4:1 static curve gives 9 dB reduction at 12 dB over threshold");
    expect(Compressor::reduction(-30, -24, 4) == 0, "below-knee signal stays uncompressed");
    Parameters p; p.highpass = 0; p.threshold = -24; p.ratio = 4; p.attackMs = .5f;
    Compressor c; c.prepare(48000, p);
    const auto output = settle(c, Compressor::gain(-12));
    expect(std::abs(Compressor::db(output) - (-21)) < .04f, "steady-state gain matches threshold and ratio");
    p.bypass = true; c.set(p); auto bypass = settle(c, .2f);
    expect(std::abs(bypass - .2f) < .00001f, "bypass returns dry audio after smooth transition");
    p.bypass = false; p.mix = 0; c.prepare(48000, p);
    expect(std::abs(settle(c, .2f) - .2f) < .000001f, "0% mix is sample-exact dry");
    p.mix = 100; c.prepare(48000, p);
    float l = .4f, r = .1f;
    for (int i = 0; i < 48000; ++i) { l = .4f; r = .1f; c.process(l, r, l, r); }
    expect(std::abs(l / r - 4) < .00001f, "stereo linking preserves left/right balance");
    p.mode = 0; c.prepare(48000, p); settle(c, .5f, 4800);
    float fixedStart = c.gainReduction(); settle(c, 0, 4800); float fixedEnd = c.gainReduction();
    expect(fixedStart > 10 && fixedEnd < fixedStart * .3f, "fixed mode releases quickly");
    p.mode = 1; p.releaseMs = 2000; c.prepare(48000, p); settle(c, .5f, 4800);
    float manualStart = c.gainReduction(); settle(c, 0, 4800);
    expect(c.gainReduction() > manualStart * .9f, "manual mode obeys slow release");
    p.mode = 2; p.attackMs = 100; p.releaseMs = 1000; c.prepare(48000, p);
    settle(c, .5f, 480); settle(c, 0, 9600); float shortTail = c.gainReduction();
    c.prepare(48000, p); settle(c, .5f, 48000); settle(c, 0, 9600);
    expect(c.gainReduction() > shortTail * 2, "combined mode has more optical memory after sustained signals");
    for (double sr : {22050.0, 44100.0, 48000.0, 96000.0, 192000.0}) {
        p.threshold = -60; p.ratio = 10; p.makeup = 30; p.warmth = 100; p.highpass = 250;
        c.prepare(sr, p); bool finite = true;
        for (int i = 0; i < 24000; ++i) { float a = std::sin(i * .17f) * 4, b = -a; c.process(a, b, a, b); finite = finite && std::isfinite(a) && std::isfinite(b); }
        expect(finite, "extreme settings remain finite across sample rates");
    }
    c.prepare(48000, p); l = std::numeric_limits<float>::quiet_NaN(); r = std::numeric_limits<float>::infinity(); c.process(l, r, l, r);
    expect(std::isfinite(l) && std::isfinite(r), "invalid input samples do not poison processor state");
    c.prepare(48000, p); expect(settle(c, 0) == 0, "silence stays silent");
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
