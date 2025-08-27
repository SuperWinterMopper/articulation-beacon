#pragma once

#include <JuceHeader.h>
#include "ScoreData.h"
#include "Utils.h"
#include "GaussianSmoother.h"
#include "TargetPack.h"

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
    juce::ValueTree scoreState;
    int exerciseDataIndex;
    juce::dsp::WindowingFunction<float> hannWindow;

    //Graph properties
    int renderWidthInPixels = 1 << 10;

    //===================================
    // Below are DSP state management variables
    juce::dsp::FFT fft;
    std::array<float, fftSize> fifo;
    std::array<float, fftSize * 2> fftData;
    //std::array<float, fftSize> freqBins;
    int fifoIndex = 0;
    
    static int sampleRate;
    int64_t totalSamplesProcessed = 0;
    float secondsPerSample;
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
    std::array<int64_t, fluxSize> fluxTimeFifo;   // absolute sample index (center of FFT window, minus smoothing delay)
    std::array<int64_t, fluxSize> fluxTimeData;   // unwrapped times for the scan window
    std::array<float, fluxSize> ampFifo; //amp value for each index in fluxFifo
    std::array<float, fluxSize> ampData; 
    static constexpr int gaussDelayFrames = GaussianFIR<7>::delay();

    int fluxFifoIndex = 0;
    int fluxCount = 0; //counts up to fluxHopLength to know when to measure the fluxs next

    //Gaussian smoother
    GaussianFIR<7> fluxSmoother{ 1.0f };

    //Optimized metadata for this articulation exercise
    struct MetaData {
        float onsetThresh = 1.0f;
        int bpm = -1;
        float minSecondsBetweenNotes = 100.0f;
        int64_t minSamplesBetweenNotes = 0; // will be set later
        float sustainThresholdValue = -1.0f;
        int minFramesBetweenNotes = 0; // NEW: same interval, but in flux frames
    };
    MetaData metaData;

    //for approximating normalization of spectral flux
    struct LeakyMaxNormalizer {
        float sampleRate = 0;
        int   hopLength = 512;     // flux step in samples
        float tauSeconds = 20.f;
        float headroom = 10.f;    // matches optimal peak ~0.10 -> headroom=10
        float eps = 1e-6f;
        float g = 1e-3f;

        // Call once per spectral-flux value. Returns normalized flux in [~0..1].
        float update(float x) {
            const float dt = hopLength / sampleRate;
            const float a = std::exp(-dt / tauSeconds);   // 0<a<1
            g = std::max(x, g * a);
            return x / (headroom * g + eps);
        }

        void reset(float g0 = 1e-3f) { g = g0; }
    };
    LeakyMaxNormalizer norm;

    struct ArticulationWindow {
        int64_t onsetSample = 0;
        int onsetSampleIndex = 0;
        int64_t sustainSample = 0;
        int sustainSampleIndex = 0; 
        std::vector<float> flux;
        std::vector<float> amps;
        std::vector<float> cents;  // this is added but not used yet
    };
    //This denotes how many samples before onset and after sustain we should include. 
    //Will not always be of this size due to how we analyze the fifo but usually will be this.
    int detectionPaddingSize = 10;
    std::vector<ArticulationWindow> snapShots;
    std::vector<ArticulationWindow> targetArticulations;
    std::unordered_set<int64_t> foundOnsetSamples; //to easily look up if an articulation has been included in snapShots yet
    std::unordered_set<int64_t> renderedSnapShots;
    //===================================

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property);
    void getMagnitudeSpectrogram(float* a_fftData, float* res);
    void processInputSample(float sample);
    bool processFluxSampleAndIfScan(float rawFlux, float amp);
    void performAnalysis();
    float computeFluxValue(float* cur, float* prev);
    void performFluxScan();
    void copyFluxFifoToData();
    void renderSnapShotGraph(int64_t onsetSample);
    void updateMetaData();

    //for reading in binary data
    void loadTargetPack();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Graph)
};