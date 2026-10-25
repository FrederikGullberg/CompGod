#pragma once
#include "PluginProcessor.h"

class CompGodLook final : public juce::LookAndFeel_V4 {
public:
    CompGodLook();
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
    void drawLinearSlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider::SliderStyle, juce::Slider&) override;
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawComboBox(juce::Graphics&, int, int, bool, int, int, int, int, juce::ComboBox&) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Label* createSliderTextBox(juce::Slider&) override;
};

class CompGodEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit CompGodEditor(CompGodProcessor&);
    ~CompGodEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    void choosePreset(int);
    CompGodProcessor& processor;
    CompGodLook look;
    juce::TooltipWindow tooltips {this, 600};
    std::array<juce::Slider, 8> sliders;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 8> attachments;
    juce::ComboBox presets;
    juce::TextButton previous {"<"}, next {">"}, bypass {"BYPASS"}, external {"EXT"};
    std::array<juce::TextButton, 3> modes;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment, externalAttachment;
    std::unique_ptr<juce::ParameterAttachment> modeAttachment;
    std::array<float, 240> history {};
    int historyPosition = 0;
    float input = -90, output = -90, reduction = 0;
    float scale = 1;
    std::array<float, 9> presetValues {};
    int displayedProgram = 0;
    bool edited = false;
    void capturePresetValues();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CompGodEditor)
};
