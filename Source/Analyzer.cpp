/*
==============================================================================

    Analyzer.cpp
    Created: 21 Sep 2023 4:36:45pm
    Author:  Nic Becker

==============================================================================
*/

#include <JuceHeader.h>
#include "Analyzer.h"
#include "PluginProcessor.h"

//==============================================================================
Analyzer::Analyzer(PluginProcessor& p) : processorRef (p)
{
  // In your constructor, you should add any child components, and
  // initialise any special settings that your component needs.
  scopeSize = PluginProcessor::scopeSize;
  scopeData.resize(PluginProcessor::scopeSize);
  startTimerHz (30);
}

Analyzer::~Analyzer()
{
  stopTimer();
}

void Analyzer::paint (juce::Graphics& g)
{
  /* This demo code just fills the component's background and
     draws some placeholder text to get you started.

     You should replace everything in this method with your own
     drawing code..
  */

  g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
  drawFrame (g);

//  g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
//
//  g.setColour (juce::Colours::red);
//  g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
//
//  g.setColour (juce::Colours::blue);
//  g.setFont (14.0f);
//  g.drawText ("NewComponentTest", getLocalBounds(),
//      juce::Justification::centred, true);   // draw some placeholder text
}

void Analyzer::resized()
{
  // This method is where you should set the bounds of any child
  // components that your component contains..
}

void Analyzer::drawNextFrameOfSpectrum()
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

void Analyzer::drawFrame (juce::Graphics& g)
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

void Analyzer::timerCallback()
{
  if (processorRef.nextFFTBlockReady.get())
  {
      drawNextFrameOfSpectrum();
      processorRef.nextFFTBlockReady.set(false);
      repaint();
  }
}


