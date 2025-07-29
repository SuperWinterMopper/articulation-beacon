
#include <JuceHeader.h>
#include "Metronome.h"

//==============================================================================
Metronome::Metronome(int bpm, juce::ValueTree a_scoreState) : bpm(bpm), scoreState(a_scoreState)
{
    formatManager.registerBasicFormats();

    juce::String path = "Resources/metSample.wav";
    juce::File metronomeSample = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile(path);

    jassert(metronomeSample.existsAsFile());

    auto formatReader = formatManager.createReaderFor(metronomeSample);
    metronomeSampleLength = int(formatReader->lengthInSamples);

    metronomeSamplePtr.reset(new juce::AudioFormatReaderSource(formatReader, true));
    metronomeSamplePtr->setNextReadPosition(0);
}

void Metronome::prepareToPlay(int samplesPerBlock, double appSampleRate)
{
    sampleRate = appSampleRate;
    interval = int(60.0 / bpm * sampleRate);

    if (metronomeSamplePtr != nullptr)
    {
        metronomeSamplePtr->prepareToPlay(samplesPerBlock, sampleRate);
        DBG("Metronome file loaded");
    }
}

void Metronome::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{

    const int numSamps = bufferToFill.numSamples;
    // how many samples we’d already processed into this beat
    int prevRem = totalSamples % interval;
    totalSamples += numSamps;

    // did we cross the beat boundary?
    if (prevRem + numSamps >= interval)
    {
        //====================================
        curBeat++; //increment beat

        //IMPORTANT: if the beat at 0 (we've finished our 4 clicks in), we start the video to play along with it
        if (curBeat == 1) scoreState.setProperty(isVideoPlaying, true, nullptr);
        //====================================

        // where, in this block, the click should start
        int clickOffset = interval - prevRem;

        // restart the reader
        metronomeSamplePtr->setNextReadPosition(0);

        int chunk1Len = std::min(metronomeSampleLength, numSamps - clickOffset);
        juce::AudioSourceChannelInfo chunk1{ bufferToFill.buffer, bufferToFill.startSample + clickOffset, chunk1Len };
        metronomeSamplePtr->getNextAudioBlock(chunk1);

        int remaining = metronomeSampleLength - chunk1Len;
        if (remaining > 0) {
            int chunk2Len = std::min(remaining, numSamps);
            juce::AudioSourceChannelInfo chunk2{ bufferToFill.buffer, bufferToFill.startSample, chunk2Len };
            metronomeSamplePtr->getNextAudioBlock(chunk2);
        }
    }
}

void Metronome::reset()
{
    totalSamples = 0; //reset
    curBeat = -4; //reset
    metronomeSamplePtr->setNextReadPosition(0);
}