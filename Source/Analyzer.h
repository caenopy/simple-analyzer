/*
==============================================================================

    Analyzer.h
    Created: 21 Sep 2023 4:36:45pm
    Author:  Nic Becker

==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/

class PluginProcessor; // Forward declaration

class Analyzer  : public juce::Component,
                  public juce::Timer
{
public:
    explicit Analyzer(PluginProcessor&);
    ~Analyzer() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void drawNextFrameOfSpectrum();
    void drawFrame (juce::Graphics& g);

private:
    PluginProcessor& processorRef;
    int scopeSize;
    std::vector<float> scopeData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Analyzer)
};
