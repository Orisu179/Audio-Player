#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class PositionLine  : public juce::Component, public juce::Timer
{
public:
    PositionLine(juce::AudioTransportSource&, juce::Label&);
    ~PositionLine() override;
    void paint (juce::Graphics&) override;

private:
    void timerCallback() override;
    juce::AudioTransportSource& transportSource;
    juce::Label& timeLabel;
    juce::String strTime;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PositionLine)
};
