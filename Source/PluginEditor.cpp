#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setOpaque(true);
    startTimerHz (30);
    setSize (700, 500);
}

PluginEditor::~PluginEditor()
{
    stopTimer();
}

//==============================================================================
void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
    drawFrame (g);
}

void PluginEditor::resized()
{
    // lay out the positions of your components
}

void PluginEditor::drawNextFrameOfSpectrum()
{
    auto mindB = -100.0f;
    auto maxdB =    0.0f;

    auto smoothedFftData = processorRef.smoothedFftData;

    for (int i = 0; i < scopeSize; ++i)
    {
        auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) PluginProcessor::scopeSize) * 0.2f);
        auto fftDataIndex = juce::jlimit (0, PluginProcessor::fftSize / 2, (int) (skewedProportionX * (float) PluginProcessor::fftSize * 0.5f));
        auto level = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (smoothedFftData[fftDataIndex])
                                                                 - juce::Decibels::gainToDecibels ((float) PluginProcessor::fftSize)),
            mindB, maxdB, 0.0f, 1.0f);

        scopeData[i] = level;
    }
}

void PluginEditor::drawFrame (juce::Graphics& g)
{
    auto width  = getLocalBounds().getWidth();
    auto height = getLocalBounds().getHeight();

    juce::Path path;
    path.preallocateSpace(8 + scopeSize * 3);
    path.startNewSubPath (
        juce::jmap<float> (0, 0, scopeSize - 1, 0, width),
        juce::jmap<float> (scopeData[0], 0.0f, 1.0f, (float) height, 0.0f)
    );

    for (int i = 1; i < scopeSize; ++i)
    {
        path.lineTo (
                juce::jmap<float> (i,     0, scopeSize - 1, 0, width),
                juce::jmap<float> (scopeData[i],     0.0f, 1.0f, (float) height, 0.0f)
        );
//        g.drawLine ({
//                        (float) juce::jmap (i - 1, 0, scopeSize - 1, 0, width),
//                        juce::jmap (scopeData[i - 1], 0.0f, 1.0f, (float) height, 0.0f),
//                        (float) juce::jmap (i,     0, scopeSize - 1, 0, width),
//                        juce::jmap (scopeData[i],     0.0f, 1.0f, (float) height, 0.0f) },
//                2.0f);
    }

    path.lineTo (getLocalBounds().getBottomRight().toFloat());
    path.lineTo (getLocalBounds().getBottomLeft().toFloat());
    path.closeSubPath();
    g.fillPath (path.createPathWithRoundedCorners(2));


}

void PluginEditor::timerCallback()
{
    if (processorRef.nextFFTBlockReady)
    {
        drawNextFrameOfSpectrum();
        processorRef.nextFFTBlockReady = false;
        repaint();
    }
}
