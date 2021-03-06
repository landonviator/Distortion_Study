/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Pedal_iR_PrototyperAudioProcessorEditor::Pedal_iR_PrototyperAudioProcessorEditor (Pedal_iR_PrototyperAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    shadowProperties.radius = 24;
    shadowProperties.offset = juce::Point<int> (-1, 3);
    dialShadow.setShadowProperties (shadowProperties);
                
    //Input slider
    addAndMakeVisible(inputSlider);
    inputSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    inputSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 32);
    inputSlider.setTextValueSuffix(" dB");
    inputSlider.setColour(0x1001400, juce::Colour::fromFloatRGBA(1, 1, 1, 0.25f));
    inputSlider.setColour(0x1001700, juce::Colour::fromFloatRGBA(1, 1, 1, 0.0f));
    inputSlider.setColour(0x1001500, juce::Colour::fromFloatRGBA(0, 0, 0, 0.25f));
    inputSlider.setLookAndFeel(&customDial);
    inputSlider.setComponentEffect(&dialShadow);
    inputSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, inputSliderId, inputSlider);
    inputSlider.setRange(-24.0, 24.0, 0.25);
    
    addAndMakeVisible(inputLabel);
    inputLabel.setText("Input", juce::dontSendNotification);
    inputLabel.setJustificationType(juce::Justification::centred);
    inputLabel.setColour(0x1000281, juce::Colour::fromFloatRGBA(1, 1, 1, 0.25f));
    inputLabel.attachToComponent(&inputSlider, false);
                
    //Trim slider
    addAndMakeVisible(trimSlider);
    trimSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    trimSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 32);
    trimSlider.setTextValueSuffix(" dB");
    trimSlider.setColour(0x1001400, juce::Colour::fromFloatRGBA(1, 1, 1, 0.25f));
    trimSlider.setColour(0x1001700, juce::Colour::fromFloatRGBA(1, 1, 1, 0.0f));
    trimSlider.setColour(0x1001500, juce::Colour::fromFloatRGBA(0, 0, 0, 0.25f));
    trimSlider.setLookAndFeel(&customDial);
    trimSlider.setComponentEffect(&dialShadow);
    trimSliderAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.treeState, trimSliderId, trimSlider);
    trimSlider.setRange(-24.0, 24.0, 0.25);
    
    addAndMakeVisible(trimLabel);
    trimLabel.setText("Trim", juce::dontSendNotification);
    trimLabel.setJustificationType(juce::Justification::centred);
    trimLabel.setColour(0x1000281, juce::Colour::fromFloatRGBA(1, 1, 1, 0.25f));
    trimLabel.attachToComponent(&trimSlider, false);
    
    setSize (400, 300);
}

Pedal_iR_PrototyperAudioProcessorEditor::~Pedal_iR_PrototyperAudioProcessorEditor()
{
}

//==============================================================================
void Pedal_iR_PrototyperAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    juce::Rectangle<int> background = AudioProcessorEditor::getLocalBounds();
        g.setGradientFill(juce::ColourGradient::vertical(juce::Colour::fromFloatRGBA(0.18f, 0.20f, 0.24f, 1.0), 300 * .01, juce::Colour::fromFloatRGBA(0.12f, 0.14f, 0.18f, 1.0), 300 * .99));
        g.fillRect(background);
        g.setColour (juce::Colours::white.darker(1.0));
        g.setFont (36.0f);
        g.drawFittedText ("Pedal Simulator", 0, 24, AudioProcessorEditor::getWidth(), AudioProcessorEditor::getHeight(), juce::Justification::centredTop, 1);
}

void Pedal_iR_PrototyperAudioProcessorEditor::resized()
{
    //Master bounds object
    juce::Rectangle<int> bounds = getLocalBounds();
                        
    //first column of gui
    juce::FlexBox flexboxColumnOne;
    flexboxColumnOne.flexDirection = juce::FlexBox::Direction::column;
    flexboxColumnOne.flexWrap = juce::FlexBox::Wrap::noWrap;
    flexboxColumnOne.alignContent = juce::FlexBox::AlignContent::stretch;
                       
    juce::Array<juce::FlexItem> itemArrayColumnOne;
    itemArrayColumnOne.add(juce::FlexItem(bounds.getWidth() / 4, bounds.getHeight() / 2, inputSlider).withMargin(juce::FlexItem::Margin(bounds.getHeight() * .35, 0, 0, 0)));
                       
    flexboxColumnOne.items = itemArrayColumnOne;
    flexboxColumnOne.performLayout(bounds.removeFromLeft(bounds.getWidth() / 2));
    /* ============================================================================ */
                   
    //second column of gui
    juce::FlexBox flexboxColumnTwo;
    flexboxColumnTwo.flexDirection = juce::FlexBox::Direction::column;
    flexboxColumnTwo.flexWrap = juce::FlexBox::Wrap::noWrap;
    flexboxColumnTwo.alignContent = juce::FlexBox::AlignContent::stretch;
                       
    juce::Array<juce::FlexItem> itemArrayColumnTwo;
    itemArrayColumnTwo.add(juce::FlexItem(bounds.getWidth() / 4, bounds.getHeight() / 2, trimSlider).withMargin(juce::FlexItem::Margin(bounds.getHeight() * .35, 0, 0, 0)));
                       
    flexboxColumnTwo.items = itemArrayColumnTwo;
    flexboxColumnTwo.performLayout(bounds.removeFromLeft(bounds.getWidth()));
}
