#pragma once

#include <JuceHeader.h>

class Graph : public juce::Component, private juce::ValueTree::Listener
{
public:
    Graph(juce::ValueTree scoreState);
    ~Graph() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    static constexpr auto fftOrder = 11;
    static constexpr auto fftSize = 1 << fftOrder;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();

private:
    juce::dsp::FFT fft;
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    juce::ValueTree scoreState;

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Graph)
};
