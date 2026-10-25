#include "PluginProcessor.h"
#include <iostream>
class RenderTimer final : public juce::Timer {
public:
    std::function<void()> renderFrame;
    void timerCallback() override { renderFrame(); }
};
int failures = 0;
void expect(bool condition, const char* message) {
    std::cout << (condition ? "PASS " : "FAIL ") << message << '\n'; if (!condition) ++failures;
}
void parameter(CompGodProcessor& p, const char* id, float value) {
    auto* param = p.state.getParameter(id); param->setValueNotifyingHost(param->convertTo0to1(value));
}
int main(int argc, char** argv) {
    juce::ScopedJuceInitialiser_GUI gui;
    CompGodProcessor processor;
    processor.setPlayConfigDetails(2, 2, 48000, 256); processor.prepareToPlay(48000, 256);
    juce::AudioBuffer<float> b(2, 256); juce::MidiBuffer midi;
    processor.setCurrentProgram(1); parameter(processor, "threshold", -31.5f);
    juce::MemoryBlock saved; processor.getStateInformation(saved);
    CompGodProcessor restored; restored.setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
    expect(std::abs(restored.parameters().threshold + 31.5f) < .01f && restored.getCurrentProgram() == 1, "state restores edited values and preset index");
    processor.setStateInformation("bad", 3);
    expect(std::abs(processor.parameters().threshold + 31.5f) < .01f, "malformed state is ignored");
    processor.setCurrentProgram(0); parameter(processor, "highpass", 0);
    for (int block = 0; block < 400; ++block) {
        for (int ch = 0; ch < 2; ++ch) for (int i = 0; i < 256; ++i) b.setSample(ch, i, .25f * std::sin((block * 256 + i) * .13f));
        processor.processBlock(b, midi);
    }
    expect(processor.reductionMeter.load() > 3 && b.getMagnitude(0, 256) < .25f, "plugin processes audio and meters compression");
    parameter(processor, "mix", 0);
    for (int block = 0; block < 100; ++block) { for (int ch = 0; ch < 2; ++ch) juce::FloatVectorOperations::fill(b.getWritePointer(ch), .1f, 256); processor.processBlock(b, midi); }
    expect(std::abs(b.getSample(0, 200) - .1f) < .0001f, "plugin parallel mix reaches dry signal");
    // Real host bus routing, including a mono external sidechain.
    auto layout = processor.getBusesLayout(); layout.inputBuses.set(1, juce::AudioChannelSet::mono());
    expect(processor.setBusesLayout(layout), "external mono sidechain layout accepted");
    parameter(processor, "mix", 100); parameter(processor, "external", 1); processor.prepareToPlay(48000, 256);
    juce::AudioBuffer<float> sideBuffer(3, 256);
    for (int block = 0; block < 400; ++block) {
        juce::FloatVectorOperations::fill(sideBuffer.getWritePointer(0), .01f, 256);
        juce::FloatVectorOperations::fill(sideBuffer.getWritePointer(1), .01f, 256);
        juce::FloatVectorOperations::fill(sideBuffer.getWritePointer(2), .7f, 256);
        processor.processBlock(sideBuffer, midi);
    }
    expect(processor.externalActive.load() && processor.reductionMeter.load() > 10, "external detector drives gain reduction independently of main input");
    for (int i = 0; i < 6; ++i) { processor.setCurrentProgram(i); expect(processor.getCurrentProgram() == i, "factory preset loads"); }
    const juce::File bundle(COMPGOD_VST3_PATH);
    expect(bundle.exists(), "built VST3 bundle exists");
    if (bundle.exists()) {
        juce::VST3PluginFormat format;
        juce::OwnedArray<juce::PluginDescription> descriptions;
        format.findAllTypesForFile(descriptions, bundle.getFullPathName());
        expect(descriptions.size() == 1, "VST3 host discovers one CompGod effect");
        if (!descriptions.isEmpty()) {
            juce::String error;
            auto instance = format.createInstanceFromDescription(*descriptions[0], 48000, 256, error);
            expect(instance != nullptr, "built VST3 instantiates in a real plugin host");
            if (instance) {
                instance->setPlayConfigDetails(2, 2, 48000, 256); instance->prepareToPlay(48000, 256);
                bool finite = true;
                for (int block = 0; block < 100; ++block) {
                    for (int ch = 0; ch < 2; ++ch) for (int i = 0; i < 256; ++i) b.setSample(ch, i, .25f * std::sin((block * 256 + i) * .13f));
                    instance->processBlock(b, midi);
                    for (int ch = 0; ch < 2; ++ch) for (int i = 0; i < 256; ++i) finite = finite && std::isfinite(b.getSample(ch, i));
                }
                expect(finite && b.getMagnitude(0, 256) < .25f, "hosted VST3 processes finite, compressed stereo audio");
                juce::MemoryBlock hostState; instance->getStateInformation(hostState); instance->setStateInformation(hostState.getData(), static_cast<int>(hostState.getSize()));
                expect(hostState.getSize() > 0, "hosted VST3 returns restorable state");
                std::unique_ptr<juce::AudioProcessorEditor> hostedEditor(instance->createEditorIfNeeded());
                expect(hostedEditor != nullptr && hostedEditor->getWidth() >= 880, "hosted VST3 creates its native editor");
                hostedEditor.reset(); instance->releaseResources();
            } else std::cout << error << '\n';
        }
    }
    if (argc > 1) {
        processor.setCurrentProgram(1);
        std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
        // Dispatch native timers while feeding a repeatable level pattern to draw real meter history.
        layout.inputBuses.set(1, juce::AudioChannelSet::disabled()); processor.setBusesLayout(layout);
        parameter(processor, "external", 0); processor.prepareToPlay(48000, 256);
        int frame = 0;
        RenderTimer renderTimer;
        renderTimer.renderFrame = [&] {
            const float amplitude = .045f + .24f * std::pow(.5f + .5f * std::sin(frame * .12f), 3.0f);
            for (int block = 0; block < 6; ++block) {
                for (int ch = 0; ch < 2; ++ch) for (int i = 0; i < 256; ++i) b.setSample(ch, i, amplitude * std::sin(i * .14f));
                processor.processBlock(b, midi);
            }
            if (++frame >= 180) {
                renderTimer.stopTimer();
                juce::MessageManager::getInstance()->stopDispatchLoop();
            }
        };
        renderTimer.startTimer(34);
        juce::MessageManager::getInstance()->runDispatchLoop();
        for (auto* child : editor->getChildren()) if (auto* slider = dynamic_cast<juce::Slider*>(child)) {
            if (slider->getName() == "release") {
                expect(std::abs(slider->getValueFromText("1.5 s") - 1500) < .01, "release text entry converts seconds to milliseconds");
                expect(slider->getTextFromValue(650).contains("ms"), "release display includes time units");
            }
            if (slider->getName() == "threshold") {
                const auto previousValue = slider->getValue();
                slider->setValue(-28.5, juce::sendNotificationSync);
                expect(std::abs(processor.parameters().threshold + 28.5f) < .01f, "native knob updates the host parameter");
                slider->setValue(previousValue, juce::sendNotificationSync);
            }
        }
        // Render the actual native editor, not a separate HTML mockup.
        editor->setSize(1100, 760);
        auto image = editor->createComponentSnapshot(editor->getLocalBounds(), true, 1.0f);
        juce::File file(juce::String::fromUTF8(argv[1])); file.deleteFile(); auto stream = file.createOutputStream();
        expect(stream != nullptr && juce::PNGImageFormat().writeImageToStream(image, *stream), "native editor screenshot written");
        editor->setSize(880, 608);
        auto compactImage = editor->createComponentSnapshot(editor->getLocalBounds(), true, 1.0f);
        auto smallFile = file.getSiblingFile("CompGod-compact.png"); smallFile.deleteFile(); auto smallStream = smallFile.createOutputStream();
        expect(smallStream != nullptr && juce::PNGImageFormat().writeImageToStream(compactImage, *smallStream), "compact native editor screenshot written");
    }
    return failures == 0 ? 0 : 1;
}
