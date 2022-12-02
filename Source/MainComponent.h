#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  :  public juce::AudioAppComponent,
                        public juce::ChangeListener,
                        public juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void changeListenerCallback (juce::ChangeBroadcaster* source) override;
    void thumbnailChanged();
    //==============================================================================
    void paint(juce::Graphics& g) override;
    void paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void paintIfLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    //==============================================================================
    void resized() override;
    void timerCallback() override;


private:
    //==============================================================================
    // Your private member variables go here...
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Pausing,
        Paused,
        Stopping
    };

    TransportState state;
    void changeState(TransportState newState);

    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();

    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;

    juce::Label timeLabel;
    juce::String strTime;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    juce::AudioThumbnailCache thumbnailCache; //passes argument to AudioThumbnail
    juce::AudioThumbnail thumbnail;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
