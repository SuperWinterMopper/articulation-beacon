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
    static constexpr int fftOrder = 11;
    static constexpr int fftSize = 1 << fftOrder; //each buffer is roughly 46ms at sampleRate = 44100
    static constexpr int numBins = fftSize / 2 + 1;
    static constexpr int overlap = 4;
    static constexpr int hopLength = fftSize / overlap;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();
    void reset();

    void timerCallback() override;

private:
    juce::dsp::FFT fft;
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    std::array<float, fftSize> freqBins;
    int fifoIndex = 0;
    bool nextFFTBlockReady = false;
    
    int sampleRate;
    int64_t totalSamplesProcessed = 0;
    float secondsPerSample;
    
    juce::ValueTree scoreState;
    int exerciseDataIndex;
    ExerciseDataStruct metaData;
    juce::dsp::WindowingFunction<float> hannWindow;
    juce::dsp::WindowingFunction<float> 

    //===================================
    // Below are DSP state management variables
    int count = 0; //counts up to hopLength to know when next to take FFT

    //the previous magnitude spectrogram, used for finding spectral flux.
    std::vector<float> prevMags{ numBins };
    std::vector<float> newMags{ numBins };
    bool prevMagsComputed = false;


    //For maintaining flux values
    static constexpr int fluxOrder = 9;
    static constexpr int fluxSize = 1 << fluxOrder;
    static constexpr int fluxHopLength = fluxSize / overlap;
    std::array<float, fluxSize> fluxFifo;
    std::array<float, fluxSize> fluxData;
    int fluxFifoIndex = 0;
    int fluxCount = 0; //counts up to fluxHopLength to know when to measure the fluxs next

    //2 block system containing consecutive 2 fftSize size chunks of amplitudes, spectral flux, and times
    enum class BuffersComputed { Zero, One, Two };
    BuffersComputed buffersComputed = BuffersComputed::Zero;
    float startA, endA, startB, endB;
    std::array<float, fftSize> ampsA, ampsB;
    std::array<float, fftSize> fluxA, fluxB;
    std::array<float, fftSize> timesA, timesB;
    //===================================

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property);
    void getMagnitudeSpectrogram(float* a_fftData, float* res);
    void processInputSample(float sample);
    bool processFluxSampleAndIfScan(float sample);
    void performAnalysis();
    float computeFluxValue(float* cur, float* prev);
    void performFluxScan();


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Graph)
};