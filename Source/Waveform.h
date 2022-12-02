#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Waveform  : public juce::Component, private juce::ChangeListener
{
public:
    Waveform(int sourceSample, juce::AudioFormatManager& formatManager, juce::AudioThumbnailCache& thumbnailCache);

    void paint (juce::Graphics&) override;
    void paintIfNoFileLoaded(juce::Graphics& g);
    void paintIfFileLoaded(juce::Graphics& g);
    void setFile(const juce::File& file);
    void changeListenerCallback(juce::ChangeBroadcaster* source);
private:
    juce::AudioThumbnail thumbnail;
    void thumbnailChanged();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Waveform)
};
