#include "PluginEditor.h"

#include <memory>
#include "PluginProcessor.h"

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p, juce::AudioProcessorValueTreeState& vts, juce::UndoManager& um, double fs)
    : AudioProcessorEditor (&p),
      processorRef (p),
      undoManager (um),
      apvts (vts),
      scope (p, fs)
{
    setWantsKeyboardFocus (true);
    juce::ignoreUnused (processorRef);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setOpaque(true);
    setSize (500, 400);
    addAndMakeVisible(scope);

    smoothTimeDial.setSliderStyle (juce::Slider::Rotary);
    smoothTimeDial.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(smoothTimeDial);
    smoothTimeAttachment = std::make_unique<SliderAttachment> (apvts, "smoothTime", smoothTimeDial);

}

PluginEditor::~PluginEditor()
{
}

//==============================================================================
void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void PluginEditor::resized()
{
    auto border = 20;

    // lay out the positions of your components
    juce::Rectangle<int> r = getLocalBounds();
    scope.setBounds(r.reduced (border).withTrimmedBottom (border * 4));

    auto dialArea = r.withTrimmedTop(15 * border);
    smoothTimeDial.setBounds (dialArea);
}

bool PluginEditor::keyPressed (const juce::KeyPress& key)
{
    const auto cmdZ = juce::KeyPress { 'z', juce::ModifierKeys::commandModifier, 0 };

    if (key == cmdZ && undoManager.canUndo())
    {
        undoManager.undo();
        return true;
    }

    const auto cmdShiftZ = juce::KeyPress { 'z', juce::ModifierKeys::commandModifier
                                                     | juce::ModifierKeys::shiftModifier, 0 };

    if (key == cmdShiftZ && undoManager.canRedo())
    {
        undoManager.redo();
        return true;
    }

    return false;
}
