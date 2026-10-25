#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace {
constexpr const char* ids[] {"threshold", "ratio", "attack", "release", "makeup", "mix", "warmth", "highpass", "mode", "bypass", "external", "unused"};
constexpr const char* presetNames[] {"Clean start", "Vocal velvet", "Bass anchor", "Drum snap", "Mix glue", "Parallel crush"};
constexpr float presets[6][9] {
    {-24, 4, 10, 400, 0, 100, 0, 80, 1},
    {-26, 3, 18, 650, 4, 100, 15, 100, 1},
    {-24, 4, 25, 300, 3, 100, 12, 45, 1},
    {-18, 4, 1, 50, 2, 100, 8, 80, 0},
    {-18, 2, 90, 800, 2, 100, 5, 120, 2},
    {-36, 8, 3, 180, 10, 40, 25, 100, 1}
};
}
CompGodProcessor::CompGodProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
        .withInput("Sidechain", juce::AudioChannelSet::stereo(), false)),
      state(*this, nullptr, "CompGodState", makeLayout()) {
    for (int i = 0; i < 11; ++i) values[static_cast<size_t>(i)] = state.getRawParameterValue(ids[i]);
}
juce::AudioProcessorValueTreeState::ParameterLayout CompGodProcessor::makeLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    auto add = [&](const char* id, const char* name, float min, float max, float step, float def, float centre = 0) {
        juce::NormalisableRange<float> range(min, max, step);
        if (centre > min && centre < max) range.setSkewForCentre(centre);
        p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(id, 1), name, range, def));
    };
    add("threshold", "Threshold (dBFS)", -60, 0, .1f, -24);
    add("ratio", "Ratio", 2, 10, .1f, 4);
    add("attack", "Attack / combined memory (ms)", .5f, 300, .1f, 10, 20);
    add("release", "Release (ms)", 50, 10000, 1, 400, 500);
    add("makeup", "Makeup (dB)", -30, 30, .1f, 0);
    add("mix", "Mix (%)", 0, 100, 1, 100);
    add("warmth", "Warmth (%)", 0, 100, 1, 0);
    add("highpass", "Detector high-pass (Hz)", 0, 250, 1, 80);
    p.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("mode", 1), "Timing mode", juce::StringArray{"Fixed", "Manual", "Combined"}, 1));
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("bypass", 1), "Bypass", false));
    p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("external", 1), "External sidechain", false));
    return p;
}
compgod::Parameters CompGodProcessor::parameters() const {
    compgod::Parameters p;
    p.threshold = values[0]->load(); p.ratio = values[1]->load();
    p.attackMs = values[2]->load(); p.releaseMs = values[3]->load();
    p.makeup = values[4]->load(); p.mix = values[5]->load();
    p.warmth = values[6]->load(); p.highpass = values[7]->load();
    p.mode = static_cast<int>(values[8]->load()); p.bypass = values[9]->load() > .5f;
    return p;
}
void CompGodProcessor::prepareToPlay(double sr, int) { compressor.prepare(sr, parameters()); }
bool CompGodProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
    const auto main = l.getMainInputChannelSet();
    if ((main != juce::AudioChannelSet::mono() && main != juce::AudioChannelSet::stereo()) || main != l.getMainOutputChannelSet()) return false;
    if (l.inputBuses.size() > 1) {
        const auto sc = l.getChannelSet(true, 1);
        if (!sc.isDisabled() && sc != juce::AudioChannelSet::mono() && sc != juce::AudioChannelSet::stereo()) return false;
    }
    return true;
}
void CompGodProcessor::processBlock(juce::AudioBuffer<float>& b, juce::MidiBuffer&) { run(b, false); }
void CompGodProcessor::processBlockBypassed(juce::AudioBuffer<float>& b, juce::MidiBuffer&) { run(b, true); }
void CompGodProcessor::run(juce::AudioBuffer<float>& buffer, bool hostBypass) {
    juce::ScopedNoDenormals noDenormals;
    auto main = getBusBuffer(buffer, true, 0);
    if (main.getNumChannels() < 1) return;
    auto p = parameters(); p.bypass = p.bypass || hostBypass; compressor.set(p);
    auto side = getBusBuffer(buffer, true, 1);
    const bool ext = values[10]->load() > .5f && side.getNumChannels() > 0;
    externalActive.store(ext);
    float peakIn = 0, peakOut = 0, peakGr = 0;
    auto* l = main.getWritePointer(0);
    auto* r = main.getNumChannels() > 1 ? main.getWritePointer(1) : nullptr;
    for (int i = 0; i < main.getNumSamples(); ++i) {
        float left = l[i], right = r ? r[i] : left;
        peakIn = std::max(peakIn, std::max(std::abs(left), std::abs(right)));
        const float sl = ext ? side.getSample(0, i) : left;
        const float sr = ext ? side.getSample(side.getNumChannels() > 1 ? 1 : 0, i) : right;
        compressor.process(left, right, sl, sr, (r || (ext && side.getNumChannels() > 1)) ? 2 : 1);
        l[i] = left; if (r) r[i] = right;
        peakOut = std::max(peakOut, std::max(std::abs(left), std::abs(right)));
        peakGr = std::max(peakGr, compressor.gainReduction());
    }
    // Time-based meter falloff stays consistent across host block sizes.
    const float fall = static_cast<float>(buffer.getNumSamples() / getSampleRate()) * 24.0f;
    inputMeter.store(std::max(compgod::Compressor::db(peakIn), inputMeter.load() - fall));
    outputMeter.store(std::max(compgod::Compressor::db(peakOut), outputMeter.load() - fall));
    reductionMeter.store(std::max(peakGr, reductionMeter.load() - fall));
}
const juce::String CompGodProcessor::getProgramName(int i) { return presetNames[juce::jlimit(0, 5, i)]; }
float CompGodProcessor::factoryValue(int preset, int parameter) { return presets[juce::jlimit(0, 5, preset)][juce::jlimit(0, 8, parameter)]; }
void CompGodProcessor::setCurrentProgram(int i) {
    i = juce::jlimit(0, 5, i); program.store(i);
    // Starting points, not calibrated replicas of the manual's clock positions.
    for (int j = 0; j < 9; ++j) {
        auto* param = state.getParameter(ids[j]);
        param->beginChangeGesture();
        param->setValueNotifyingHost(param->convertTo0to1(presets[i][j]));
        param->endChangeGesture();
    }
}
void CompGodProcessor::getStateInformation(juce::MemoryBlock& b) {
    auto tree = state.copyState(); tree.setProperty("program", program.load(), nullptr);
    if (auto xml = tree.createXml()) copyXmlToBinary(*xml, b);
}
void CompGodProcessor::setStateInformation(const void* data, int size) {
    if (auto xml = getXmlFromBinary(data, size)) if (xml->hasTagName(state.state.getType())) {
        const auto tree = juce::ValueTree::fromXml(*xml);
        state.replaceState(tree);
        program.store(juce::jlimit(0, 5, static_cast<int>(tree.getProperty("program", 0))));
    }
}
juce::AudioProcessorEditor* CompGodProcessor::createEditor() { return new CompGodEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new CompGodProcessor(); }
