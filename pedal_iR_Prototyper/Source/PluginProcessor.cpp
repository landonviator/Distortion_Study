/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Pedal_iR_PrototyperAudioProcessor::Pedal_iR_PrototyperAudioProcessor()
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

Pedal_iR_PrototyperAudioProcessor::~Pedal_iR_PrototyperAudioProcessor()
{
}

//==============================================================================
const juce::String Pedal_iR_PrototyperAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Pedal_iR_PrototyperAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Pedal_iR_PrototyperAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Pedal_iR_PrototyperAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Pedal_iR_PrototyperAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Pedal_iR_PrototyperAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Pedal_iR_PrototyperAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Pedal_iR_PrototyperAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Pedal_iR_PrototyperAudioProcessor::getProgramName (int index)
{
    return {};
}

void Pedal_iR_PrototyperAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Pedal_iR_PrototyperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    
    inputProcessor.prepare(spec);
    inputProcessor.reset();
    
    convolutionProcessor.prepare(spec);
    convolutionProcessor.reset();
    
    trimProcessor.prepare(spec);
    trimProcessor.reset();
            
    convolutionProcessor.loadImpulseResponse
        (BinaryData::pedalAt12_wav,
         BinaryData::pedalAt12_wavSize,
         juce::dsp::Convolution::Stereo::yes,
         juce::dsp::Convolution::Trim::yes, 0,
         juce::dsp::Convolution::Normalise::yes);
    
    firstAtan.prepare(spec);
    firstAtan.reset();
    
    firstAtan.functionToUse = juce::dsp::FastMathApproximations::tanh;
    
    secondAtan.prepare(spec);
    secondAtan.reset();
    
    secondAtan.functionToUse = juce::dsp::FastMathApproximations::tanh;
}

void Pedal_iR_PrototyperAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Pedal_iR_PrototyperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
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

void Pedal_iR_PrototyperAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    auto* rawInput = treeState.getRawParameterValue(inputSliderId);
    auto* rawTrim = treeState.getRawParameterValue(trimSliderId);


    juce::dsp::AudioBlock<float> audioBlock {buffer};
    
    convolutionProcessor.process(juce::dsp::ProcessContextReplacing<float> (audioBlock));
    
    inputProcessor.setGainDecibels(*rawInput);
    inputProcessor.process(juce::dsp::ProcessContextReplacing<float> (audioBlock));
    
    firstAtan.process(juce::dsp::ProcessContextReplacing<float> (audioBlock));
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* inputData = buffer.getReadPointer (channel);
        auto* outputData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); sample++) {

            outputData[sample] = (-18 * pow(inputData[sample], 3)) + (23 * pow(inputData[sample], 2)) - (5 * inputData[sample]);

        }
    }
    
    
    secondAtan.process(juce::dsp::ProcessContextReplacing<float> (audioBlock));
}

//==============================================================================
bool Pedal_iR_PrototyperAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Pedal_iR_PrototyperAudioProcessor::createEditor()
{
    return new Pedal_iR_PrototyperAudioProcessorEditor (*this);
}

//==============================================================================
void Pedal_iR_PrototyperAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Pedal_iR_PrototyperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout Pedal_iR_PrototyperAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    params.reserve(2);
    
    auto inputParam = std::make_unique<juce::AudioParameterFloat>(inputSliderId, inputSliderName, -15.0f, 7.0f, 0.0f);
    auto trimParam = std::make_unique<juce::AudioParameterFloat>(trimSliderId, trimSliderName, -24.0f, 24.0f, 0.0f);

    params.push_back(std::move(inputParam));
    params.push_back(std::move(trimParam));
    
    return { params.begin(), params.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Pedal_iR_PrototyperAudioProcessor();
}
