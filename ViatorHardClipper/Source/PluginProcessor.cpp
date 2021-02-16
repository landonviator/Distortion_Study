/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ViatorHardClipperAudioProcessor::ViatorHardClipperAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
treeState (*this, nullptr, "PARAMETER", createParameterLayout())
#endif
{
}

ViatorHardClipperAudioProcessor::~ViatorHardClipperAudioProcessor()
{
}

juce::AudioProcessorValueTreeState::ParameterLayout ViatorHardClipperAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    params.reserve(3);
    
    
    auto inputParam = std::make_unique<juce::AudioParameterFloat>(inputSliderId, inputSliderName, -24, 24, 0);
    auto driveParam = std::make_unique<juce::AudioParameterFloat>(driveSliderId, driveSliderName, 0, 24, 0);
    auto trimParam = std::make_unique<juce::AudioParameterFloat>(trimSliderId, trimSliderName, -24, 24, 0);
    
    params.push_back(std::move(inputParam));
    params.push_back(std::move(driveParam));
    params.push_back(std::move(trimParam));
    

    
    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String ViatorHardClipperAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ViatorHardClipperAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ViatorHardClipperAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ViatorHardClipperAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ViatorHardClipperAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ViatorHardClipperAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ViatorHardClipperAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ViatorHardClipperAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ViatorHardClipperAudioProcessor::getProgramName (int index)
{
    return {};
}

void ViatorHardClipperAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ViatorHardClipperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    
    inputGainProcessor.prepare(spec);
    outputGainProcessor.prepare(spec);
}

void ViatorHardClipperAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ViatorHardClipperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ViatorHardClipperAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    juce::dsp::AudioBlock<float> audioBlock (buffer);

    auto* rawInput = treeState.getRawParameterValue(inputSliderId);
    auto* rawDrive = treeState.getRawParameterValue(driveSliderId);
    auto* rawTrim = treeState.getRawParameterValue(trimSliderId);
    
    //This way the input knob controls the input and the drive/thresh. Makes it feel like using a single drive knob
    float driveScaled = scaleRange(*rawDrive, 0.0f, 24.0f, 0.9f, 0.0001f);
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* outputData = buffer.getWritePointer (channel);
        auto* inputData = buffer.getReadPointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); sample++) {
            
            if (inputData[sample] * pow(10, *rawInput * 0.05) >= driveScaled) {
                outputData[sample] = driveScaled;
                
            } else if (inputData[sample] * pow(10, *rawInput * 0.05) <= (driveScaled * -1)){
                outputData[sample] = driveScaled * -1;

            } else {
                outputData[sample] = inputData[sample] * pow(10, *rawInput * 0.05);

            }
        }
    }
    outputGainProcessor.setGainDecibels(*rawTrim);
    outputGainProcessor.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
}

float ViatorHardClipperAudioProcessor::scaleRange(float input, float inputLow, float inputHigh, float outputLow, float outputHigh){
    return ((input - inputLow) / (inputHigh - inputLow)) * (outputHigh - outputLow) + outputLow;

}

//==============================================================================
bool ViatorHardClipperAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ViatorHardClipperAudioProcessor::createEditor()
{
    return new ViatorHardClipperAudioProcessorEditor (*this);
}

//==============================================================================
void ViatorHardClipperAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ViatorHardClipperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ViatorHardClipperAudioProcessor();
}
