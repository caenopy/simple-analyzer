#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::Identifier fftID {"fftPlot"};
static juce::String smoothTime{"smoothTime"}; // uniform initialization of juce::String

static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat> (juce::ParameterID(smoothTime, 1),
                                                            "Smooth Time (ms)",
                                                            juce::NormalisableRange<float>(0, 500, 1),
                                                            250));

    return layout;
}

//==============================================================================
PluginProcessor::PluginProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
// The analyzer has no outputs, so we can comment this out:
//                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
      apvts (*this, &undoManager, "Parameters", createParameterLayout()),
      forwardFFT (fftOrder),
      window (fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    apvts.addParameterListener (smoothTime, this);
    for (int i = 0; i < 2 * fftSize; ++i)
    {
        smoothedFftData[i] = 0;
        fftData[i] = 0;
    }
}

PluginProcessor::~PluginProcessor()
{
}

//==============================================================================
const juce::String PluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PluginProcessor::getCurrentProgram()
{
    return 0;
}

void PluginProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String PluginProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void PluginProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void PluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);

    fs = sampleRate;
    preparedToPlay = true;
}

void PluginProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    preparedToPlay = false;
}

bool PluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
//  #if JucePlugin_IsMidiEffect
//    juce::ignoreUnused (layouts);
//    return true;
//  #else
//    // This is the place where you check if the layout is supported.
//    // In this template code we only support mono or stereo.
//    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
//     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
//        return false;
//
//    // This checks if the input layout matches the output layout
//   #if ! JucePlugin_IsSynth
//    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
//        return false;
//   #endif
//
//    return true;
//  #endif

    // For analyzer Check if the layout has mono or stereo input and no output
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (not layouts.getMainOutputChannelSet().isDisabled())
        return false;

    return true;

}

void PluginProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    if (not preparedToPlay)
        return;

    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = 2; //getTotalNumInputChannels();
    auto totalNumOutputChannels = 0; //getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
//    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
//        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

    // Get left channel
    auto *channelData = buffer.getReadPointer(0);

    // Accumulate in FIFO buffer
    for (auto i = 0; i < buffer.getNumSamples(); i++)
    {
        // Push next sample into FIFO
        if (fifoIndex < fftSize)
            fifo[fifoIndex++] = channelData[i];

        if (fifoIndex == fftSize)
        {
            if (not nextFFTBlockReady.get())
            {
                // Zero temp FFT buffer
                juce::zeromem (fftData, sizeof (fftData));
                // Copy audio buffer into fftData for processing
                memcpy (fftData, fifo, sizeof (fifo));
                // Reset FIFO buffer index
                fifoIndex = 0;
                // Apply windowing function and do FFT
                window.multiplyWithWindowingTable (fftData, fftSize);
                forwardFFT.performFrequencyOnlyForwardTransform (fftData);

                // Smooth FFT data for visualization
                for (int n = 0; n < fftSize / 2; n++)
                {
                    smoothedFftData[n] = leak * smoothedFftData[n] + (1 - leak) * fftData[n];
                }

                //            for (int n = 0; n < fftSize; n++)
                //            {
                //                std::cout << smoothedFftData[n] << ", ";
                //            }
                //            std::cout << "\n";

                nextFFTBlockReady = true;
            }
        }
    }
}

//==============================================================================
bool PluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PluginProcessor::createEditor()
{
    // Generic GUI for prototyping:
//    return new juce::GenericAudioProcessorEditor (*this);

     return new PluginEditor (*this, apvts);
}

//==============================================================================
void PluginProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void PluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

void PluginProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    if (parameterID == smoothTime) {
        leak = newValue < .1 ? 0.0 : static_cast<float> (std::exp (-(fftSize) / (newValue * 0.001 * fs)));
        jassert(leak <= 1 && leak >= 0);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginProcessor();
}
