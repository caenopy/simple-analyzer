#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), scope (p)
{
    juce::ignoreUnused (processorRef);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setOpaque(true);
    setSize (700, 500);
    addAndMakeVisible(scope);

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
    // lay out the positions of your components
    juce::Rectangle<int> r = getLocalBounds();
    scope.setBounds(r.reduced(50).withTrimmedBottom(50));
}
