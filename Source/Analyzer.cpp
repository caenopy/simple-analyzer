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
Analyzer::Analyzer(PluginProcessor& p, double samplingRate) : processorRef (p), fs(samplingRate)
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
    auto mindB = -80.0f;
    auto maxdB =    0.0f;

    auto smoothedFftData = processorRef.smoothedFftData;
    float nyquist = fs * 0.5f;
    float minFrequency = 20.0f;  // Starting from 20Hz

    for (int i = 0; i < scopeSize; ++i)
    {
        float proportion = (float)i / (float)PluginProcessor::scopeSize;
        float freq = minFrequency * std::pow(nyquist / minFrequency, proportion);

        // Calculate the index for this frequency in the FFT data
        auto fftDataIndex = juce::jlimit(0, PluginProcessor::fftSize / 2, (int)(freq / nyquist * PluginProcessor::fftSize * 0.5f));

        // Previously, we scaled the horizontal axis using the following skewed logarithmic scaling:
        //  auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) PluginProcessor::scopeSize) * 0.2f);
        //  auto fftDataIndex = juce::jlimit (0, PluginProcessor::fftSize / 2, (int) (skewedProportionX * (float) PluginProcessor::fftSize * 0.5f));

        auto level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(smoothedFftData[fftDataIndex])
                                                                 - juce::Decibels::gainToDecibels((float)PluginProcessor::fftSize)),
            mindB, maxdB, 0.0f, 1.0f);

        scopeData[i] = level;
    }
}

void Analyzer::drawGrid(juce::Graphics& g, float width, float height, float mindB, float maxdB)
{
    // Colors and styles
    g.setColour(juce::Colours::grey);
    g.setOpacity(0.5f);
    float lineThickness = 1.5f;

    // Determine the Nyquist frequency (half the sampling rate)
    float nyquist = fs * 0.5f;
    float minFrequency = 20.0f;  // Starting from 20Hz, which is a common minimum for audio applications

    // Starting frequency for drawing the grid
    float baseFreq = 10.0f;

    while (baseFreq < nyquist)
    {
      // Draw major division
      drawVerticalLineForFrequency(g, baseFreq, height, width, height, nyquist, minFrequency, lineThickness);

      // Draw minor divisions
      for (int multiplier = 2; multiplier < 10 && baseFreq * multiplier <= nyquist; ++multiplier)
      {
          drawVerticalLineForFrequency(g, baseFreq * multiplier, height, width, height, nyquist, minFrequency, lineThickness * 0.7f);  // Reduced line thickness for minor divisions
      }

      baseFreq *= 10.0f;
    }

    // Draw horizontal lines for dB values
    float dbInterval = 10.0f; // Adjust as needed

    for (float db = mindB; db <= maxdB; db += dbInterval)
    {
      float y = juce::jmap<float>(db, mindB, maxdB, (float) height, 0.0f);
      g.drawLine(0.0f, y, (float) width, y, lineThickness * 0.7f);
    }
}

void Analyzer::drawVerticalLineForFrequency(juce::Graphics& g, float freq, float level, int width, int height, float nyquist, float minFrequency, float lineThickness)
{
    float logX;
    float x;

    if (freq < minFrequency)
    {
      return;
    } else if (freq == 0.0)
    {
      x = 0;
    } else
    {
      logX = (std::log(freq) - std::log(minFrequency)) / (std::log(nyquist) - std::log(minFrequency));
      x = logX * width;
    }

    g.drawLine(x, (float) height, x, (float) height - level, lineThickness);
}

void Analyzer::drawSpectrum(juce::Graphics& g, float width, float height, float mindB, float maxdB)
{
    int numFFTPoints = PluginProcessor::fftSize / 2;
    auto smoothedFftData = processorRef.smoothedFftData;
    float nyquist = fs * 0.5f;
    float minFrequency = 20.0f;  // Starting from 20Hz

    for (int i = 0; i < numFFTPoints; ++i)
    {
      float proportion = (float)i / (float)numFFTPoints;
      float freq = proportion * nyquist;

      // Draw the spectrum using vertical lines
      float level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(smoothedFftData[i])
                                                                - juce::Decibels::gainToDecibels((float)PluginProcessor::fftSize)),
          mindB, maxdB, 0.0f, (float)getLocalBounds().getHeight());

      drawVerticalLineForFrequency(g, freq, level, width, height, nyquist, minFrequency, 1.5);
    }
}

void Analyzer::drawOutline(juce::Graphics& g, float width, float height, float mindB, float maxdB)
{
    int numFFTPoints = PluginProcessor::fftSize / 2;
    auto smoothedFftData = processorRef.maxSmoothedFftData;
    float nyquist = fs * 0.5f;
    float minFrequency = 20.0f;  // Starting from 20Hz

    juce::Path outlinePath;  // This will store the outline of the spectrum

    for (int i = 0; i < numFFTPoints; ++i)
    {
      float proportion = (float)i / (float)numFFTPoints;
      float freq = proportion * nyquist;

      // dB level sets vertical position
      float level = juce::jmap(juce::jlimit(mindB, maxdB, juce::Decibels::gainToDecibels(smoothedFftData[i])
                                                                - juce::Decibels::gainToDecibels((float)PluginProcessor::fftSize)),
          mindB, maxdB, 0.0f, (float)height);

      // Horizontal position uses logarithmic scaling
      float logX;
      float x;

      if (freq < minFrequency)
      {
          x = 0;
      } else
      {
          logX = (std::log(freq) - std::log(minFrequency)) / (std::log(nyquist) - std::log(minFrequency));
          x = logX * width;
      }

      // Add the point to the outline path
      if (i == 0)  // If it's the first point, start a new sub-path
          outlinePath.startNewSubPath(x, height - level);
      else
          outlinePath.lineTo(x, height - level);
    }

    // Create a rounded version of the outline path
    juce::Path roundedOutline = outlinePath.createPathWithRoundedCorners(20.0f);  // Adjust the rounding radius as needed

    // Draw the rounded outline (only the top edge)
    g.strokePath(roundedOutline, juce::PathStrokeType(2.0f));  // Adjust the stroke type as needed

    // Fill in outline
    roundedOutline.lineTo (getLocalBounds().getBottomRight().toFloat());
    roundedOutline.lineTo (getLocalBounds().getBottomLeft().toFloat());
    roundedOutline.closeSubPath();
    g.fillPath (roundedOutline);
}


void Analyzer::drawFrame(juce::Graphics& g)
{
    auto width  = getLocalBounds().getWidth();
    auto height = getLocalBounds().getHeight();
    float mindB = -80.0f; // Adjust as needed
    float maxdB = 0.0f;   // Adjust as needed

//    juce::Path path;
//    path.preallocateSpace(8 + scopeSize * 3);
//    path.startNewSubPath (
//        juce::jmap<float> (0, 0, scopeSize - 1, 0, width),
//        juce::jmap<float> (scopeData[0], 0.0f, 1.0f, (float) height, 0.0f)
//    );
//
//    for (int i = 1; i < scopeSize; ++i)
//    {
//      path.lineTo (
//          juce::jmap<float> (i,     0, scopeSize - 1, 0, width),
//          juce::jmap<float> (scopeData[i],     0.0f, 1.0f, (float) height, 0.0f)
//      );
//      //        g.drawLine ({
//      //                        (float) juce::jmap (i - 1, 0, scopeSize - 1, 0, width),
//      //                        juce::jmap (scopeData[i - 1], 0.0f, 1.0f, (float) height, 0.0f),
//      //                        (float) juce::jmap (i,     0, scopeSize - 1, 0, width),
//      //                        juce::jmap (scopeData[i],     0.0f, 1.0f, (float) height, 0.0f) },
//      //                2.0f);
//    }
//
//    path.lineTo (getLocalBounds().getBottomRight().toFloat());
//    path.lineTo (getLocalBounds().getBottomLeft().toFloat());
//    path.closeSubPath();
//    g.fillPath (path.createPathWithRoundedCorners(2));

    // First draw the grid
    drawGrid(g, width, height, mindB, maxdB);

    // Change color
    g.setColour(juce::Colours::grey);

    // Then draw spectrum outline
    drawOutline(g, width, height, mindB, maxdB);

    // Change color
    g.setColour(juce::Colours::white);

    // Now plot the spectrum
    drawSpectrum(g, width, height, mindB, maxdB);


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


