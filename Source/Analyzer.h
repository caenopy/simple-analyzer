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
class Analyzer  : public juce::Component
{
public:
  Analyzer();
  ~Analyzer() override;

  void paint (juce::Graphics&) override;
  void resized() override;

private:
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Analyzer)
};
