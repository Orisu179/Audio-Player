#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Spectrum  : public juce::Component, private juce::Timer
{
public:
    Spectrum();
    //~Spectrum() override;

    void pushNextSampleIntoFifo (float sample) noexcept;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    enum
    {
        fftOrder = 11,
        fftSize = 1 << fftOrder,
        scopeSize = 512
    };
private:
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    bool loaded;

    float fifo [fftSize];
    float fftData [2 * fftSize];
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    float scopeData [scopeSize];

    void paintIfFileLoaded(juce::Graphics& g);
    void paintIfNoFileLoaded(juce::Graphics& g);

    void drawNextFrameSpectrum();
    void drawFrame (juce::Graphics& g);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Spectrum)
};
