#pragma once

#include <JuceHeader.h>

class Graph : public juce::AudioAppComponent
{
public:
    Graph();
    ~Graph() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    static constexpr auto fftOrder = 11;
    static constexpr auto fftSize = 1 << fftOrder;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

private:
    juce::dsp::FFT fft;
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Graph)
};
