#pragma once

#include "PluginProcessor.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor

{
public:
    PluginEditor (PluginProcessor& p,
                  juce::AudioProcessorValueTreeState& vts,
                  juce::UndoManager& um,
                  double fs);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    bool keyPressed (const juce::KeyPress& key) override;

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor& processorRef;
    juce::UndoManager& undoManager;
    juce::AudioProcessorValueTreeState& apvts;

    Analyzer scope;
    juce::Slider smoothTimeDial;

    std::unique_ptr<SliderAttachment> smoothTimeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
