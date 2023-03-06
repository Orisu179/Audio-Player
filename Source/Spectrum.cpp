#include <JuceHeader.h>
#include "Spectrum.h"

//==============================================================================
Spectrum::Spectrum() :forwardFFT (fftOrder),
                        window (fftSize, juce::dsp::WindowingFunction<float>::hann)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    startTimerHz(60);
}

//Spectrum::~Spectrum()
//{
//}

void Spectrum::paint (juce::Graphics& g)
{
    paintIfFileLoaded(g);
}

void Spectrum::paintIfFileLoaded(juce::Graphics &g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setOpacity(1.0f);
    drawFrame(g);
}

void Spectrum::paintIfNoFileLoaded(juce::Graphics &g)
{
    g.setColour (juce::Colours::black);
    g.fillRect(getLocalBounds());   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText ("Spectrum", getLocalBounds(),
                juce::Justification::centred, true);
}

void Spectrum::pushNextSampleIntoFifo(float sample) noexcept
{
    if(fifoIndex == fftSize){
        if (!nextFFTBlockReady){
            juce::zeromem (fftData, sizeof(fftData));
            memcpy (fftData, fifo, sizeof(fifo));
            nextFFTBlockReady = true;
        }
        fifoIndex = 0;
    }
    fifo[fifoIndex++] = sample;
}

void Spectrum::drawNextFrameSpectrum()
{
    window.multiplyWithWindowingTable (fftData, fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform (fftData);
    auto mindB = -100.0f;
    auto maxdB = 0.0f;
    for(int i{0}; i<scopeSize; ++i)
    {
        auto skewedProportionX = 1.0f - std::exp (std::log (1.0f - (float) i / (float) scopeSize) * 0.2f);
        auto fftDataIndex = juce::jlimit (0, fftSize / 2, (int) (skewedProportionX * (float) fftSize * 0.5f));
        auto level = juce::jmap (juce::jlimit (mindB, maxdB, juce::Decibels::gainToDecibels (fftData[fftDataIndex])
                                                             - juce::Decibels::gainToDecibels ((float) fftSize)), mindB, maxdB, 0.0f, 1.0f);
        scopeData[i] = level;
    }
}
void Spectrum::drawFrame(juce::Graphics& g)
{
    for(int i {0}; i<scopeSize; ++i)
    {
        auto width = getLocalBounds().getWidth();
        auto height = getLocalBounds().getHeight();
        g.drawLine ({ (float) juce::jmap (i-1, 0, scopeSize -1, 0, width),
                      juce::jmap(scopeData[i-1], 0.0f, 1.0f, (float) height, 0.0f),
                      (float) juce::jmap(i, 0, scopeSize-1, 0, width),
                      juce::jmap(scopeData[i], 0.0f, 1.0f, (float)height, 0.0f)
        });
    }
}

void Spectrum::timerCallback()
{
    if(nextFFTBlockReady)
    {
        drawNextFrameSpectrum();
        nextFFTBlockReady = false;
        repaint();
    }
}