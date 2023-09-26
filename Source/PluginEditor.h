#pragma once

#include "PluginProcessor.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor

{
public:
    explicit PluginEditor (PluginProcessor&, juce::AudioProcessorValueTreeState&);
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor& processorRef;
    juce::AudioProcessorValueTreeState& apvts;

    Analyzer scope;
    juce::Slider smoothTimeDial;

    std::unique_ptr<SliderAttachment> smoothTimeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
