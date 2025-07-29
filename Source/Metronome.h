#pragma once

#include <JuceHeader.h>
#include "ScoreData.h"
#include "Utils.h"

//==============================================================================
/*
*/
class Metronome 
{
public:
    Metronome(int bpm, juce::ValueTree a_scoreState);

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void reset();
    void prepareToPlay(int samplesPerBlock, double sampleRate);
    void setBPM(int a_bpm) { bpm = (double) a_bpm; interval = int(60.0 / a_bpm * sampleRate);
    };

private:
    //running counter of how many samples have been processed so far in this 1 click-cycle
    int totalSamples{ 0 };
    double sampleRate{ 0 };

    //the current count of the metronome in the measure. Metronome always plays 4 beats before starting, hence -4
    int curBeat = -4;

    int metronomeSampleLength = 0;

    //number of samples in between clicks
    int interval{ 0 };
    double bpm{ defaultBPM };

    juce::ValueTree scoreState;

    juce::AudioFormatManager formatManager;
    std::unique_ptr <juce::AudioFormatReaderSource> metronomeSamplePtr{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Metronome)
};