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
    explicit Analyzer(PluginProcessor&, double);
    ~Analyzer() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

    void drawNextFrameOfSpectrum();
    void drawGrid(juce::Graphics& g, float width, float height, float mindB, float maxdB);
    void drawSpectrum(juce::Graphics& g, float width, float height, float mindB, float maxdB);
    void drawOutline(juce::Graphics& g, float width, float height, float mindB, float maxdB);
    static void drawVerticalLineForFrequency(juce::Graphics& g, float freq, float level, int width, int height, float nyquist, float minFrequency, float lineThickness);
    void drawFrame (juce::Graphics& g);

private:
    PluginProcessor& processorRef;
    double fs;

    int scopeSize;
    std::vector<float> scopeData;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Analyzer)
};
