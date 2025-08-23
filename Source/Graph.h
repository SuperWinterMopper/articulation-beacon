#pragma once

#include <JuceHeader.h>
#include "ScoreData.h"
#include "Utils.h"

class Graph : public juce::Component, private juce::ValueTree::Listener, private juce::Timer
{
public:
    Graph(juce::ValueTree scoreState, int exerciseDataIndex);
    ~Graph() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    static constexpr auto fftOrder = 11;
    static constexpr auto fftSize = 1 << fftOrder;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();

    void timerCallback() override;

private:
    juce::dsp::FFT fft;
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;

    juce::ValueTree scoreState;
    int exerciseDataIndex;
    ExerciseDataStruct metaData;

    //===================================
    // Below are DSP state management variables
    bool search_sustain = false;


    //===================================

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property);
    void performAnalysis();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Graph)
};