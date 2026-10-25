#pragma once
#include <JuceHeader.h>
#include "Compressor.h"

class CompGodProcessor final : public juce::AudioProcessor {
public:
    CompGodProcessor();
    void prepareToPlay(double, int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "CompGod"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0; }
    int getNumPrograms() override { return 6; }
    int getCurrentProgram() override { return program.load(); }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    juce::AudioProcessorParameter* getBypassParameter() const override { return state.getParameter("bypass"); }
    compgod::Parameters parameters() const;
    static float factoryValue(int preset, int parameter);
    juce::AudioProcessorValueTreeState state;
    std::atomic<float> inputMeter {-90}, outputMeter {-90}, reductionMeter {0};
    std::atomic<bool> externalActive {false};
private:
    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();
    void run(juce::AudioBuffer<float>&, bool hostBypass);
    compgod::Compressor compressor;
    std::array<std::atomic<float>*, 12> values {};
    std::atomic<int> program {0};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompGodProcessor)
};
