#include "MainComponent.h"
#include <JuceHeader.h>

//==============================================================================
MainComponent::MainComponent() : state(Stopped),
                                 thumbnailCache (5),
                                 waveform(512, formatManager, thumbnailCache),
                                 positionLine(transportSource, timeLabel)
{
    // Make sure you set the size of the component after
    // you add any child components.


    addAndMakeVisible(&openButton);
    openButton.setButtonText ("Open... ");
    openButton.onClick = [this] { openButtonClicked(); };

    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play");
    playButton.onClick = [this] { playButtonClicked(); };
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    playButton.setEnabled(false);

    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop");
    stopButton.onClick = [this] { stopButtonClicked(); };
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    stopButton.setEnabled(false);

//    addAndMakeVisible(&showButton);
//    showButton.setButtonText("Show Audio Spectrum");
//    showButton.onClick = [this] { showButtonClicked(); };
//    showButton.setEnabled(false);

    addAndMakeVisible(&timeLabel);
    timeLabel.setJustificationType(juce::Justification::centred);
    timeLabel.attachToComponent(&openButton, false);

    addAndMakeVisible(&waveform);
    addAndMakeVisible(&positionLine);
    addAndMakeVisible(&spectrum);

    setSize (600, 800);

    formatManager.registerBasicFormats();
    transportSource.addChangeListener(this);

    setAudioChannels (0, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    if(readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
    }else
        transportSource.getNextAudioBlock(bufferToFill);

    if(bufferToFill.buffer -> getNumChannels() > 0 ){
        auto* channelData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);

        for(auto i{0}; i<bufferToFill.numSamples; ++i){
            spectrum.pushNextSampleIntoFifo (channelData[i]);
        }
    }
}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        if(transportSource.isPlaying())
            changeState(Playing);
        else if(state == Stopping || state == Playing)
            changeState(ropped);
        else if(state == Pausing)
            changeState(Paused);
    }
}
//==============================================================================
void MainComponent::resized()
{
    openButton.setBounds(10, 25, getWidth() - 20, 20);
    playButton.setBounds(10, 55, getWidth() - 20, 20);
    stopButton.setBounds(10, 85, getWidth() - 20, 20);
    //showButton.setBounds(10, 115, getWidth() - 20, 20);
    juce::Rectangle<int> thumbnailBounds(10, 150, getWidth() - 20, 300);
    juce::Rectangle<int> spectrumBounds(10, 470 , getWidth() - 20, getHeight() - 490);
    waveform.setBounds(thumbnailBounds);
    positionLine.setBounds(thumbnailBounds);
    spectrum.setBounds(spectrumBounds);
}

void MainComponent::changeState(TransportState newState)
{
    if(state != newState)
    {
        state = newState;

        switch(state)
        {
            case Stopped:
                playButton.setButtonText("Play");
                stopButton.setButtonText("Stop");
                stopButton.setEnabled(false);
                transportSource.setPosition(0.0);
                positionLine.stopTimer();
                break;

            case Starting:
                transportSource.start();
                positionLine.startTimer(40);
                break;

            case Pausing:
                transportSource.stop();
                positionLine.stopTimer();
                break;

            case Paused:
                playButton.setButtonText("Resume");
                stopButton.setButtonText("Return to Zero");
                break;

            case Playing:
                playButton.setButtonText("Pause");
                stopButton.setButtonText("Stop");
                stopButton.setEnabled(true);
                break;

            case Stopping:
                transportSource.stop();
                break;
        }
    }
}

void MainComponent::openButtonClicked() {
    chooser = std::make_unique<juce::FileChooser> ("Select a Wave file to play...",
                                                   juce::File{},
                                                   "*.wav; *.mp3; *.flac");
    auto chooserFlags = juce::FileBrowserComponent::openMode
                        | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync (chooserFlags, [this] (const juce::FileChooser& fc)
    {
        auto file = fc.getResult();

        if (file != juce::File{})
        {
            auto* reader = formatManager.createReaderFor (file);

            if (reader != nullptr)
            {
                auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
                waveform.setFile(file);
                playButton.setEnabled(true);
                playButton.setButtonText("play");
                stopButton.setButtonText("stop");
                readerSource.reset(newSource.release());
            }
        }
    });
}

void MainComponent::playButtonClicked()
{
    if(state == Stopped || state == Paused)
        changeState(Starting);
    else if(state == Playing)
        changeState(Pausing);
}

void MainComponent::stopButtonClicked()
{
    if(state == Paused)
        changeState(Stopped);
    else
        changeState(Stopping);
}

//void MainComponent::showButtonClicked()
//{
//
//}
