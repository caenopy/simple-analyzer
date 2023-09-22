#pragma once

#include <JuceHeader.h>
#include "Analyzer.h"

#if (MSVC)
#include "ipps.h"
#endif

class PluginProcessor : public juce::AudioProcessor,
                        private juce::AudioProcessorValueTreeState::Listener  // Listener for parameters
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void parameterChanged (const juce::String& parameterID, float newValue) override;

    enum
    {
        fftOrder  = 11,
        fftSize   = 1 << fftOrder, // 2048
        scopeSize = fftSize >> 1   // 1024
    };

    juce::Atomic<bool> nextFFTBlockReady = false;
    float smoothedFftData [2 * fftSize];
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)

    bool preparedToPlay = false;

    // Parameters
    float leak;
    double fs;

    // APVTS and Undo Manager
    juce::AudioProcessorValueTreeState apvts;
    juce::UndoManager undoManager;

    // FFT Setup
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    float fifo [fftSize];
    float fftData [2 * fftSize]; // dsp::FFT requires the size of the array passed in to be 2 * getSize().
    int fifoIndex = 0;
};
