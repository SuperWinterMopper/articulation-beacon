#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Metronome 
{
public:
    Metronome(int bpm);

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void reset();
    void prepareToPlay(int samplesPerBlock, double sampleRate);

private:
    //running counter of how many samples have been processed so far in this 1 click-cycle
    int totalSamples{ 0 };
    double sampleRate{ 0 };

    int metronomeSampleLength = 0;

    //number of samples in between clicks
    int interval{ 0 };
    double bpm{ 200.0 };
    int samplesRemaining{ 0 };

    juce::AudioFormatManager formatManager;
    std::unique_ptr <juce::AudioFormatReaderSource> metronomeSamplePtr{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Metronome)
};