#pragma once
#include <algorithm>
#include <array>
#include <cmath>

namespace compgod {
struct Parameters {
    float threshold = -24.0f, ratio = 4.0f, attackMs = 10.0f, releaseMs = 400.0f;
    float makeup = 0.0f, mix = 100.0f, warmth = 0.0f, highpass = 80.0f;
    int mode = 1; // Fixed, Manual, Combined
    bool bypass = false;
};

// Behavioral opto compressor; no claim of component-level circuit emulation.
// No heap allocation, locks, file IO or host calls in the sample path.
class Compressor {
public:
    static float db(float x) { return 20.0f * std::log10(std::max(x, 1.0e-9f)); }
    static float gain(float d) { return std::pow(10.0f, d * 0.05f); }
    static float reduction(float level, float threshold, float ratio) {
        constexpr float knee = 6.0f;
        const float over = level - threshold;
        const float slope = 1.0f - 1.0f / ratio;
        if (over <= -knee * 0.5f) return 0.0f;
        if (over >= knee * 0.5f) return over * slope;
        const float t = over + knee * 0.5f;
        return slope * t * t / (2.0f * knee);
    }
    void prepare(double sampleRate, const Parameters& p = {}) {
        sr = std::max(8000.0, sampleRate);
        current = target = p;
        smooth = 1.0f - std::exp(-1.0f / (0.02f * static_cast<float>(sr)));
        detectorDecay = coefficient(5.0f);
        fastAttack = coefficient(1.0f); fastRelease = coefficient(50.0f);
        bypassAmount = p.bypass ? 0.0f : 1.0f;
        detector = gr = fastGr = slowGr = 0;
        hpX.fill(0); hpY.fill(0); counter = 0;
        updateCoefficients();
    }
    void set(const Parameters& p) { target = p; }
    void process(float& left, float& right, float sideL, float sideR, int channels = 2) {
        auto slew = [this](float& a, float b) { a += smooth * (b - a); };
        slew(current.threshold, target.threshold); slew(current.ratio, target.ratio);
        slew(current.makeup, target.makeup); slew(current.mix, target.mix);
        slew(current.warmth, target.warmth); slew(current.highpass, target.highpass);
        slew(current.attackMs, target.attackMs); slew(current.releaseMs, target.releaseMs);
        slew(bypassAmount, target.bypass ? 0.0f : 1.0f);
        if ((counter++ & 15) == 0) updateCoefficients();
        const std::array<float, 2> side { sideL, sideR };
        float peak = 0;
        for (int ch = 0; ch < channels; ++ch) {
            const float x = std::isfinite(side[ch]) ? side[ch] : 0.0f;
            // Bypass HP at zero, but retain valid filter history for automation.
            const float y = hpAlpha * (hpY[ch] + x - hpX[ch]);
            hpX[ch] = x; hpY[ch] = std::abs(y) < 1e-20f ? 0.0f : y;
            peak = std::max(peak, std::abs(current.highpass < 1.0f ? x : y));
        }
        detector = std::max(peak, detector * detectorDecay);
        float desired = reduction(db(detector), current.threshold, current.ratio);
        fastGr = follow(fastGr, desired, fastAttack, fastRelease);
        slowGr = follow(slowGr, desired, attackCoeff, releaseCoeff);
        const float selected = target.mode == 0 ? fastGr : target.mode == 1 ? slowGr : std::max(fastGr, slowGr);
        // Tiny smoothing on the mode selector prevents discontinuities on automation.
        gr += (1.0f - fastAttack) * (selected - gr);
        const float wetGain = gain(current.makeup - gr);
        const float amount = current.mix * 0.01f * bypassAmount;
        auto render = [&](float input) {
            if (!std::isfinite(input)) input = 0;
            float wet = input * wetGain;
            // Bounded, symmetric tube-like colour. Clean mode preserves linear gain.
            const float colour = current.warmth * 0.01f;
            if (colour > 0.0001f) {
                const float drive = 1.0f + colour * 1.5f;
                const float shaped = std::tanh(wet * drive) / drive;
                wet += colour * 0.35f * (shaped - wet);
            }
            return input + amount * (wet - input);
        };
        left = render(left);
        if (channels > 1) right = render(right);
    }
    float gainReduction() const { return gr * bypassAmount; }
private:
    float coefficient(float ms) const { return std::exp(-1.0f / (ms * .001f * static_cast<float>(sr))); }
    static float follow(float state, float input, float a, float r) {
        const float result = input + (input > state ? a : r) * (state - input);
        return std::abs(result) < 1e-20f ? 0 : result;
    }
    void updateCoefficients() {
        attackCoeff = coefficient(std::clamp(current.attackMs, 0.5f, 300.0f));
        releaseCoeff = coefficient(std::clamp(current.releaseMs, 50.0f, 10000.0f));
        hpAlpha = std::exp(-6.28318530718f * std::max(1.0f, current.highpass) / static_cast<float>(sr));
    }
    double sr = 44100;
    Parameters current, target;
    std::array<float, 2> hpX {}, hpY {};
    float smooth = 0, detectorDecay = 0, detector = 0, gr = 0, fastGr = 0, slowGr = 0;
    float attackCoeff = 0, releaseCoeff = 0, fastAttack = 0, fastRelease = 0, hpAlpha = 0, bypassAmount = 1;
    unsigned int counter = 0;
};
}
