#include <JuceHeader.h>
#include "Waveform.h"

//==============================================================================
Waveform::Waveform(int sourceSample,
                   juce::AudioFormatManager& formatManager,
                   juce::AudioThumbnailCache& thumbnailCache) : thumbnail(sourceSample, formatManager, thumbnailCache)
{
    thumbnail.addChangeListener(this);
}

void Waveform::paintIfNoFileLoaded(juce::Graphics &g)
{
    g.fillAll(juce::Colours::darkgrey);
    g.setColour(juce::Colours::white);
    g.drawFittedText("No File loaded", getLocalBounds(), juce::Justification::centred, 1);
}

void Waveform::paintIfFileLoaded(juce::Graphics &g)
{
    g.setColour(juce::Colours::black);
    g.fillRect(getLocalBounds());

    g.setColour(juce::Colours::white);
    auto audioLength = static_cast<float> (thumbnail.getTotalLength());
    thumbnail.drawChannels(g, getLocalBounds(), 0.0, audioLength, 1.0f);
                                                //start time, end time, vertical zoom
}
void Waveform::paint (juce::Graphics& g)
{
    if(thumbnail.getNumChannels() == 0)
        paintIfNoFileLoaded(g);
    else
        paintIfFileLoaded(g);
}

void Waveform::setFile(const juce::File& file) {
    thumbnail.setSource(new juce::FileInputSource(file));
}

void Waveform::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if(source==&thumbnail)
        thumbnailChanged();
}

void Waveform::thumbnailChanged()
{
    repaint();
}
