#include <JuceHeader.h>
#include "PositionLine.h"

//==============================================================================
PositionLine::PositionLine(juce::AudioTransportSource& transportSource1,
                           juce::Label& timeLabel1) :
                           transportSource(transportSource1),
                           timeLabel(timeLabel1)
{
    startTimer(40);
}

PositionLine::~PositionLine()
{
    stopTimer();
}

void PositionLine::paint (juce::Graphics& g)
{
    auto audioLength = static_cast<float> (transportSource.getLengthInSeconds());
    if(audioLength > 0.0) {
        g.setColour(juce::Colours::green);
        auto audioPosition = static_cast<float> (transportSource.getCurrentPosition());
        auto drawPosition = (audioPosition / audioLength) * static_cast<float> (getWidth());
        g.drawLine(drawPosition, 0.0f, drawPosition, static_cast<float> (getHeight()), 2.0f);
    }
}

void PositionLine::timerCallback()
{
    repaint();
    juce::RelativeTime total(transportSource.getLengthInSeconds());
    juce::RelativeTime cur(transportSource.getCurrentPosition());
    if(cur.inSeconds() >= 1.0)
        strTime = cur.getDescription("0") + " / " + total.getDescription("");
    else
        strTime = "0 secs / " + total.getDescription("");
    timeLabel.setText(strTime, juce::dontSendNotification);
}


