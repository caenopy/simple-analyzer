#include "PluginEditor.h"

#include <memory>
#include "PluginProcessor.h"

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p),
      processorRef (p),
      apvts (vts),
      scope (p)
{
    juce::ignoreUnused (processorRef);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setOpaque(true);
    setSize (700, 500);
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
    auto border = 4;

    // lay out the positions of your components
    juce::Rectangle<int> r = getLocalBounds();
    scope.setBounds(r.reduced(50).withTrimmedBottom(50));

    auto dialArea = r.removeFromTop (r.getHeight() / 2);
    smoothTimeDial.setBounds (dialArea.removeFromLeft (dialArea.getWidth() / 2).reduced (border));
}
