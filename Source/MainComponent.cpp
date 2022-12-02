#include "MainComponent.h"
#include <JuceHeader.h>

//==============================================================================
MainComponent::MainComponent() : state(Stopped),
                                 thumbnailCache (5),
                                 thumbnail(512, formatManager, thumbnailCache)
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

    addAndMakeVisible(&timeLabel);
    timeLabel.setJustificationType(juce::Justification::centred);
    timeLabel.attachToComponent(&openButton, false);


    setSize (800, 600);

    formatManager.registerBasicFormats();
    thumbnail.addChangeListener(this);
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
        return;
    }

    transportSource.getNextAudioBlock(bufferToFill);
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
            changeState(Stopped);
        else if(state == Pausing)
            changeState(Paused);
    }

    if(source == &thumbnail)    thumbnailChanged();
}

void MainComponent::thumbnailChanged()
{
    repaint();
}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    juce::Rectangle<int> thumbnailBounds(10, 120, getWidth() -20, getHeight() - 120);
    if(thumbnail.getNumChannels() == 0)
        paintIfNoFileLoaded(g, thumbnailBounds);
    else
        paintIfLoaded(g, thumbnailBounds);
}

void MainComponent::paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::white);
    g.drawFittedText("No File loaded", thumbnailBounds, juce::Justification::centred, 1);
}

void MainComponent::paintIfLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    g.setColour(juce::Colours::black);
    g.fillRect(thumbnailBounds);

    g.setColour(juce::Colours::white);
    auto audioLength = static_cast<float> (thumbnail.getTotalLength());
    thumbnail.drawChannels(g, thumbnailBounds, 0.0, audioLength, 1.0f);
                                                //start time, end time, vertical zoom
    g.setColour(juce::Colours::green);

    auto audioPosition = static_cast<float> (transportSource.getCurrentPosition());
    auto drawPosition = (audioPosition/audioLength) * static_cast<float> (thumbnailBounds.getWidth())
                        + static_cast<float> (thumbnailBounds.getX());
    g.drawLine (drawPosition, static_cast<float> (thumbnailBounds.getY()), drawPosition,
                static_cast<float> (thumbnailBounds.getBottom()), 2.0f);
}

//==============================================================================
void MainComponent::resized()
{
    openButton.setBounds(10, 25, getWidth() - 20, 20);
    playButton.setBounds(10, 55, getWidth() - 20, 20);
    stopButton.setBounds(10, 85, getWidth() - 20, 20);
}

void MainComponent::timerCallback()
{
    repaint();
    juce::RelativeTime cur(transportSource.getCurrentPosition());
    juce::RelativeTime total(transportSource.getLengthInSeconds());
    if(cur.inSeconds() >= 1.0)
        strTime = cur.getDescription("0") + " / " + total.getDescription("");
    else
        strTime = "0 secs / " + total.getDescription("");
    timeLabel.setText(strTime, juce::dontSendNotification);
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
                break;

            case Starting:
                transportSource.start();
                break;

            case Pausing:
                transportSource.stop();
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
                startTimer (40);
                auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
                thumbnail.setSource(new juce::FileInputSource (file));

                playButton.setEnabled(true);
                readerSource.reset(newSource.release());
                playButton.setButtonText("Play");
                stopButton.setButtonText("Stop");
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


