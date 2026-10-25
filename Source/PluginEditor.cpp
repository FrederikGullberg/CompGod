#include "PluginEditor.h"

namespace {
const juce::Colour bg {0xff101715}, panel {0xff19221e}, inset {0xff101a15};
const juce::Colour border {0xff2a3730}, mint {0xffbaf7ce}, text {0xffedf3ed}, muted {0xff92a698}, amber {0xfff2bc87};
constexpr const char* ids[] {"threshold", "ratio", "attack", "release", "makeup", "mix", "warmth", "highpass"};
constexpr const char* labels[] {"THRESHOLD", "RATIO", "ATTACK", "RELEASE", "MAKEUP"};
juce::Font font(float size, bool bold = false) {
   #if JUCE_WINDOWS
    const juce::String family = "Segoe UI";
   #else
    const auto family = juce::Font::getDefaultSansSerifFontName();
   #endif
    return juce::Font(juce::FontOptions(family, size < 16 ? size + 2 : size, bold ? juce::Font::bold : juce::Font::plain));
}
void label(juce::Graphics& g, const juce::String& s, float x, float y, float w, float h, float size, juce::Colour colour, juce::Justification align = juce::Justification::centredLeft, bool bold = false) {
    g.setFont(font(size, bold)); g.setColour(colour); g.drawText(s, juce::Rectangle<float>(x, y, w, h), align);
}
void card(juce::Graphics& g, juce::Rectangle<float> r) {
    g.setColour(panel); g.fillRoundedRectangle(r, 16);
    g.setColour(border); g.drawRoundedRectangle(r.reduced(.5f), 16, 1);
}
}
CompGodLook::CompGodLook() {
    setColour(juce::Slider::textBoxTextColourId, text);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxHighlightColourId, mint.withAlpha(.25f));
    setColour(juce::ComboBox::textColourId, text);
    setColour(juce::PopupMenu::backgroundColourId, panel);
    setColour(juce::PopupMenu::textColourId, text);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, mint);
    setColour(juce::PopupMenu::highlightedTextColourId, bg);
    setColour(juce::TextButton::textColourOffId, muted);
    setColour(juce::TextButton::textColourOnId, bg);
    setColour(juce::TooltipWindow::backgroundColourId, mint);
    setColour(juce::TooltipWindow::textColourId, bg);
    setColour(juce::TooltipWindow::outlineColourId, border);
}
void CompGodLook::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float position, float start, float end, juce::Slider& slider) {
    auto r = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h)).reduced(12);
    const float radius = std::min(r.getWidth(), r.getHeight()) * .5f;
    auto c = r.getCentre();
    const float angle = start + position * (end - start);
    const auto accent = slider.getName() == "threshold" ? mint : juce::Colour(0xffa8bbb0);
    const float opacity = slider.isEnabled() ? 1.0f : .28f;
    for (int i = 0; i <= 24; ++i) {
        float a = start + static_cast<float>(i) / 24.0f * (end - start);
        g.setColour((i <= position * 24.0f ? accent : border).withAlpha(opacity));
        g.drawLine(c.x + std::sin(a) * (radius + 6), c.y - std::cos(a) * (radius + 6),
                   c.x + std::sin(a) * (radius + 9), c.y - std::cos(a) * (radius + 9), 1.5f);
    }
    juce::Path track, arc;
    track.addCentredArc(c.x, c.y, radius, radius, 0, start, end, true);
    arc.addCentredArc(c.x, c.y, radius, radius, 0, start, angle, true);
    g.setColour(border); g.strokePath(track, juce::PathStrokeType(3));
    g.setColour(accent.withAlpha(opacity)); g.strokePath(arc, juce::PathStrokeType(3, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    auto body = juce::Rectangle<float>(radius * 1.58f, radius * 1.58f).withCentre(c);
    g.setColour(juce::Colours::black.withAlpha(.22f)); g.fillEllipse(body.translated(0, 3));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff344238), c.x, body.getY(), juce::Colour(0xff202c25), c.x, body.getBottom(), false));
    g.fillEllipse(body); g.setColour(juce::Colour(0xff4b5d4f).withAlpha(.55f)); g.drawEllipse(body, 1);
    g.setColour(accent.withAlpha(opacity));
    g.drawLine(c.x + std::sin(angle) * radius * .42f, c.y - std::cos(angle) * radius * .42f,
               c.x + std::sin(angle) * radius * .65f, c.y - std::cos(angle) * radius * .65f, 3);
    g.setColour(accent.withAlpha(.15f * opacity)); g.fillEllipse(c.x - 4, c.y - 4, 8, 8);
}
void CompGodLook::drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float position, float, float, juce::Slider::SliderStyle, juce::Slider&) {
    const float cy = static_cast<float>(y) + h * .5f;
    g.setColour(border); g.fillRoundedRectangle(static_cast<float>(x), cy - 2, static_cast<float>(w), 4, 2);
    g.setColour(mint); g.fillRoundedRectangle(static_cast<float>(x), cy - 2, std::max(0.0f, position - x), 4, 2);
    g.fillEllipse(position - 5, cy - 5, 10, 10);
}
void CompGodLook::drawButtonBackground(juce::Graphics& g, juce::Button& b, const juce::Colour&, bool hover, bool down) {
    auto r = b.getLocalBounds().toFloat().reduced(.5f);
    g.setColour(b.getToggleState() ? mint : (down ? border.brighter(.1f) : hover ? border : inset));
    g.fillRoundedRectangle(r, 8);
    g.setColour(b.getToggleState() ? mint : border); g.drawRoundedRectangle(r, 8, 1);
}
void CompGodLook::drawComboBox(juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&) {
    g.setColour(inset); g.fillRoundedRectangle(.5f, .5f, w - 1.0f, h - 1.0f, 8);
    g.setColour(border); g.drawRoundedRectangle(.5f, .5f, w - 1.0f, h - 1.0f, 8, 1);
    juce::Path arrow; arrow.startNewSubPath(w - 22.0f, h * .45f); arrow.lineTo(w - 18.0f, h * .55f); arrow.lineTo(w - 14.0f, h * .45f);
    g.setColour(muted); g.strokePath(arrow, juce::PathStrokeType(1.5f));
}
juce::Font CompGodLook::getComboBoxFont(juce::ComboBox&) { return font(14); }
juce::Label* CompGodLook::createSliderTextBox(juce::Slider& s) {
    auto* value = juce::LookAndFeel_V4::createSliderTextBox(s);
    value->setFont(font(s.isRotary() ? 18.0f : 14.0f));
    value->setColour(juce::Label::outlineColourId, juce::Colours::transparentBlack);
    value->setColour(juce::Label::textColourId, text);
    value->setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    value->setColour(juce::TextEditor::backgroundColourId, inset);
    value->setColour(juce::TextEditor::textColourId, text);
    value->setColour(juce::TextEditor::outlineColourId, border);
    value->setColour(juce::TextEditor::focusedOutlineColourId, mint);
    return value;
}

CompGodEditor::CompGodEditor(CompGodProcessor& p) : AudioProcessorEditor(p), processor(p) {
    setLookAndFeel(&look);
    const char* tips[] {
        "Level where compression begins, in dBFS. Lower it for more gain reduction.",
        "Compression strength. At 4:1, a 4 dB rise above threshold becomes about 1 dB.",
        "Manual: attack time. Combined: optical memory; longer settings preserve a faster initial release.",
        "How slowly the gain returns. Combined mode blends a fast release with this slower tail.",
        "Output gain after compression. Match loudness when comparing with bypass.",
        "Parallel compression: blend the original signal with the processed signal.",
        "Subtle, symmetric saturation. Zero is clean; higher values add gentle harmonic colour.",
        "Filters bass in the detector only, so kicks trigger less compression. Zero turns the filter off."
    };
    const float defaults[] {-24, 4, 10, 400, 0, 100, 0, 80};
    for (size_t i = 0; i < sliders.size(); ++i) {
        auto& s = sliders[i]; s.setName(ids[i]); s.setLookAndFeel(&look);
        s.setSliderStyle(i < 5 ? juce::Slider::RotaryHorizontalVerticalDrag : juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(i < 5 ? juce::Slider::TextBoxBelow : juce::Slider::TextBoxRight, false, i < 5 ? 150 : 64, 28);
        s.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
        s.setDoubleClickReturnValue(true, defaults[i]); s.setTooltip(tips[i]);
        s.setWantsKeyboardFocus(true); s.setTitle(juce::String(ids[i]));
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.state, ids[i], s);
        s.textFromValueFunction = [i](double v) {
            if (i == 0 || i == 4) return juce::String(v, 1) + " dB";
            if (i == 1) return juce::String(v, 1) + ":1";
            if (i == 2) return juce::String(v, v < 10 ? 1 : 0) + " ms";
            if (i == 3) return v >= 1000 ? juce::String(v / 1000, 2) + " s" : juce::String(v, 0) + " ms";
            if (i == 7) return v < 1 ? juce::String("Off") : juce::String(v, 0) + " Hz";
            return juce::String(v, 0) + " %";
        };
        s.valueFromTextFunction = [i](const juce::String& t) {
            auto v = t.getDoubleValue();
            if (i == 3 && t.trim().endsWithIgnoreCase("s") && !t.containsIgnoreCase("ms")) v *= 1000;
            return v;
        };
        s.updateText();
        addAndMakeVisible(s);
    }
    for (int i = 0; i < 6; ++i) presets.addItem(processor.getProgramName(i), i + 1);
    addAndMakeVisible(presets); presets.setTooltip("Starting points. Adjust threshold to suit your recording.");
    presets.onChange = [this] { if (presets.getSelectedId() > 0) choosePreset(presets.getSelectedId() - 1); };
    for (auto* b : {&previous, &next, &bypass, &external}) addAndMakeVisible(b);
    previous.onClick = [this] { choosePreset((processor.getCurrentProgram() + 5) % 6); };
    next.onClick = [this] { choosePreset((processor.getCurrentProgram() + 1) % 6); };
    previous.setTooltip("Previous preset"); next.setTooltip("Next preset");
    bypass.setClickingTogglesState(true); external.setClickingTogglesState(true);
    bypass.setTooltip("Compare with the original signal. Output is not limited; watch the OUT meter.");
    external.setTooltip("Use the DAW's sidechain input. Falls back to internal detection if no sidechain bus is enabled.");
    bypassAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state, "bypass", bypass);
    externalAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.state, "external", external);
    const char* names[] {"Fixed", "Manual", "Combined"};
    for (size_t i = 0; i < modes.size(); ++i) {
        auto& b = modes[i]; b.setButtonText(names[i]); addAndMakeVisible(b);
        b.onClick = [this, i] { modeAttachment->setValueAsCompleteGesture(static_cast<float>(i)); };
        b.setTooltip(i == 0 ? "1 ms attack, 50 ms release. Quick, consistent control." : i == 1 ? "Use the attack and release knobs." : "1 ms attack with fast recovery and an optical memory tail. Attack sets the memory time.");
    }
    modeAttachment = std::make_unique<juce::ParameterAttachment>(*processor.state.getParameter("mode"), [this](float v) {
        for (size_t i = 0; i < modes.size(); ++i) modes[i].setToggleState(static_cast<int>(v) == static_cast<int>(i), juce::dontSendNotification);
    }, nullptr);
    modeAttachment->sendInitialUpdate();
    capturePresetValues();
    setResizable(true, true); setResizeLimits(880, 608, 1650, 1140);
    getConstrainer()->setFixedAspectRatio(1100.0 / 760.0);
    setSize(1100, 760); startTimerHz(30);
}
CompGodEditor::~CompGodEditor() { stopTimer(); setLookAndFeel(nullptr); }
void CompGodEditor::capturePresetValues() {
    displayedProgram = processor.getCurrentProgram(); edited = false;
    for (size_t i = 0; i < 9; ++i) presetValues[i] = CompGodProcessor::factoryValue(displayedProgram, static_cast<int>(i));
    presets.setSelectedId(displayedProgram + 1, juce::dontSendNotification);
}
void CompGodEditor::choosePreset(int i) { processor.setCurrentProgram(i); capturePresetValues(); }
void CompGodEditor::resized() {
    scale = getWidth() / 1100.0f;
    auto place = [this](juce::Component& c, int x, int y, int w, int h) {
        c.setTransform(juce::AffineTransform::scale(scale)); c.setBounds(x, y, w, h);
    };
    place(previous, 595, 26, 32, 36); place(presets, 635, 26, 240, 36); place(next, 883, 26, 32, 36);
    place(bypass, 939, 26, 133, 36);
    for (size_t i = 0; i < 5; ++i) place(sliders[i], 53 + static_cast<int>(i) * 207, 394, 166, 148);
    for (size_t i = 0; i < 3; ++i) place(modes[i], 48 + static_cast<int>(i) * 108, 624, 100, 36);
    place(sliders[5], 436, 626, 174, 30); place(sliders[6], 649, 626, 174, 30); place(sliders[7], 861, 626, 186, 30);
    place(external, 1004, 595, 44, 24);
}
void CompGodEditor::timerCallback() {
    input = processor.inputMeter.load(); output = processor.outputMeter.load(); reduction = processor.reductionMeter.load();
    history[static_cast<size_t>(historyPosition)] = reduction;
    historyPosition = (historyPosition + 1) % static_cast<int>(history.size());
    if (displayedProgram != processor.getCurrentProgram()) capturePresetValues();
    edited = false;
    for (size_t i = 0; i < 8; ++i) edited = edited || std::abs(presetValues[i] - processor.state.getRawParameterValue(ids[i])->load()) > .01f;
    edited = edited || presetValues[8] != processor.state.getRawParameterValue("mode")->load();
    const int mode = static_cast<int>(processor.state.getRawParameterValue("mode")->load());
    sliders[2].setEnabled(mode != 0); sliders[3].setEnabled(mode != 0);
    repaint();
}
void CompGodEditor::paint(juce::Graphics& graphics) {
    auto& g = graphics; g.fillAll(bg); g.addTransform(juce::AffineTransform::scale(scale));
    // Compact brand mark, drawn as vectors at every editor scale.
    g.setColour(mint); g.fillRoundedRectangle(28, 23, 42, 42, 12);
    juce::Path mark; mark.startNewSubPath(38, 50); mark.lineTo(36, 37); mark.lineTo(44, 42); mark.lineTo(49, 33); mark.lineTo(54, 42); mark.lineTo(62, 37); mark.lineTo(60, 50); mark.closeSubPath();
    g.setColour(bg); g.fillPath(mark);
    label(g, "CompGod", 83, 20, 200, 32, 27, text, juce::Justification::centredLeft, true);
    label(g, "OPTICAL DYNAMICS", 85, 50, 220, 18, 10, muted);
    label(g, edited ? "PRESET / MODIFIED" : "PRESET", 440, 25, 146, 36, 10, muted, juce::Justification::centredRight);
    g.setColour(border); g.drawHorizontalLine(87, 28, 1072);
    label(g, "Smooth control. Open sound.", 28, 101, 650, 37, 26, text, juce::Justification::centredLeft, true);
    label(g, "An optical soul, with room to shape it your way.", 29, 141, 650, 22, 13, muted);
    g.setColour(processor.state.getRawParameterValue("bypass")->load() > .5f ? amber : mint);
    g.fillEllipse(913, 123, 6, 6);
    label(g, bypass.getToggleState() ? "BYPASSED" : "STEREO LINKED", 928, 115, 144, 22, 11, muted, juce::Justification::centredRight);

    card(g, {28, 184, 688, 166}); card(g, {732, 184, 340, 166});
    label(g, "GAIN REDUCTION", 48, 198, 260, 22, 11, muted, juce::Justification::centredLeft, true);
    label(g, "8 seconds", 589, 198, 106, 22, 10, muted, juce::Justification::centredRight);
    const juce::Rectangle<float> plot(49, 236, 616, 92);
    for (int i = 0; i < 4; ++i) {
        float y = plot.getY() + i * plot.getHeight() / 3;
        g.setColour(border); g.drawHorizontalLine(static_cast<int>(y), plot.getX(), plot.getRight());
        label(g, juce::String(i * 6), 674, y - 7, 22, 14, 9, muted);
    }
    for (int i = 1; i < 8; ++i) { g.setColour(border.withAlpha(.45f)); g.drawVerticalLine(static_cast<int>(plot.getX() + i * plot.getWidth() / 8), plot.getY(), plot.getBottom()); }
    juce::Path line;
    for (size_t i = 0; i < history.size(); ++i) {
        const auto index = (static_cast<size_t>(historyPosition) + i) % history.size();
        const float x = plot.getX() + static_cast<float>(i) * plot.getWidth() / (history.size() - 1);
        const float y = plot.getY() + juce::jlimit(0.0f, 18.0f, history[index]) / 18.0f * plot.getHeight();
        if (i == 0) line.startNewSubPath(x, y); else line.lineTo(x, y);
    }
    auto fill = line; fill.lineTo(plot.getRight(), plot.getY()); fill.lineTo(plot.getX(), plot.getY()); fill.closeSubPath();
    g.setColour(mint.withAlpha(.08f)); g.fillPath(fill);
    g.setColour(mint); g.strokePath(line, juce::PathStrokeType(2, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    label(g, "COMPRESSION", 753, 198, 160, 22, 11, muted, juce::Justification::centredLeft, true);
    label(g, juce::String(-reduction, 1), 751, 226, 175, 61, 49, mint, juce::Justification::centredLeft, true);
    label(g, "dB", 753, 292, 80, 20, 13, muted);
    const float barX[] {940, 1002}; const float levels[] {input, output}; const char* names[] {"IN", "OUT"};
    for (int i = 0; i < 2; ++i) {
        label(g, names[i], barX[i] - 9, 205, 50, 17, 10, muted, juce::Justification::centred);
        for (int j = 0; j < 16; ++j) {
            const bool on = levels[i] > -60.0f + j * 4.0f;
            g.setColour(on ? (j > 13 ? amber : mint).withAlpha(j > 13 ? 1.0f : .8f) : border);
            g.fillRoundedRectangle(barX[i], 299.0f - j * 4.5f, 28, 3, 1);
        }
        label(g, levels[i] < -80 ? "-inf" : juce::String(levels[i], 1), barX[i] - 12, 312, 54, 18, 10, levels[i] >= 0 ? amber : muted, juce::Justification::centred);
    }
    if (output >= 0) label(g, "OVER 0 dBFS", 754, 319, 150, 16, 10, amber);

    card(g, {28, 366, 1044, 199});
    const char* captions[] {"Where it begins", "How much it holds", "Let transients through", "Let the signal breathe", "Bring the level back"};
    int timing = static_cast<int>(processor.state.getRawParameterValue("mode")->load());
    for (int i = 0; i < 5; ++i) {
        const float x = 48.0f + i * 207.0f;
        label(g, i == 2 && timing == 2 ? "MEMORY" : labels[i], x, 379, 177, 23, 11, i == 0 ? mint : muted, juce::Justification::centred, true);
        label(g, i == 2 && timing == 2 ? "Shape the release tail" : timing == 0 && (i == 2 || i == 3) ? (i == 2 ? "Fixed at 1 ms" : "Fixed at 50 ms") : captions[i], x - 5, 542, 187, 17, 10, muted, juce::Justification::centred);
        if (i < 4) { g.setColour(border); g.drawVerticalLine(static_cast<int>(x + 191), 394, 538); }
    }
    card(g, {28, 581, 370, 108}); card(g, {414, 581, 658, 108});
    label(g, "TIMING", 48, 594, 200, 23, 10, muted, juce::Justification::centredLeft, true);
    label(g, timing == 0 ? "FAST + CONSISTENT" : timing == 1 ? "YOUR ENVELOPE" : "PROGRAM DEPENDENT", 168, 594, 208, 23, 9, mint, juce::Justification::centredRight);
    label(g, "MIX", 436, 594, 120, 23, 10, muted, juce::Justification::centredLeft, true);
    label(g, "WARMTH", 649, 594, 130, 23, 10, muted, juce::Justification::centredLeft, true);
    label(g, "SIDECHAIN HP", 861, 594, 135, 23, 10, muted, juce::Justification::centredLeft, true);
    label(g, "Original / processed", 436, 661, 177, 17, 10, muted);
    label(g, "Clean / coloured", 649, 661, 177, 17, 10, muted);
    label(g, external.getToggleState() ? (processor.externalActive.load() ? "External detector active" : "No bus / using internal") : "Keep the low end moving", 861, 661, 190, 17, 10, muted);
    g.setColour(border); g.drawHorizontalLine(709, 28, 1072);
    label(g, "TIP", 29, 721, 34, 20, 10, mint, juce::Justification::centredLeft, true);
    label(g, "Start with a preset. Lower threshold until the meter feels right.", 64, 721, 725, 20, 12, muted);
    label(g, "ZERO LATENCY   /   v1.0", 836, 721, 236, 20, 10, muted, juce::Justification::centredRight);
}
