
#include <JuceHeader.h>
#include "Metronome.h"

//==============================================================================
Metronome::Metronome(int bpm, juce::ValueTree a_scoreState, int a_exerciseID) : bpm(bpm), scoreState(a_scoreState), exerciseID(a_exerciseID)
{
    formatManager.registerBasicFormats();
    scoreState.addListener(this);
    setMetCycleNumBeats();

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

void Metronome::setMetCycleNumBeats() 
{
    metCycleNumBeats = 0;
    DBG("inside setMetCycleNumBeats, the scoreState.getProperty(scoreView).toString() is " << scoreState.getProperty(scoreView).toString());
    //if the number of beats for this met cycle should be the whole exercise, then set it so
    if (scoreState.getProperty(scoreView).toString() == "Whole") {
        for (int i = 0; i < lineBeatLength[exerciseID].size(); i++)
            metCycleNumBeats += lineBeatLength[exerciseID][i];
    }
    else {
        int lineNum = static_cast<int>(scoreState.getProperty(scoreView));
        metCycleNumBeats = lineBeatLength[exerciseID][lineNum];
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

        if (scoreState.getProperty(userMode).toString() == "Hear then Play")
            hearThenPlayControl();
        else if (scoreState.getProperty(userMode).toString() == "Play")
            playControl();
        else {
            DBG("userMode HAS NOT BEEN SET PROPERLY");
            jassert(false);
        }

        //IMPORTANT: if the beat at 1 (we've finished our 4 clicks in), we start the video to play along with it
        //if (curBeat == 1) {
        //    scoreState.setProperty(isVideoPlaying, true, nullptr);
        //    scoreState.setProperty(isVideoMuted, false, nullptr);
        //}
        //else if (curBeat == metCycleNumBeats + 1) { //recording has just finished playing, user's turn to play 

        //    //this resets the video
        //    scoreState.setProperty(isVideoPlaying, false, nullptr);
        //    scoreState.setProperty(isVideoPlaying, true, nullptr);

        //    //Mute the video and begin analyzing
        //    scoreState.setProperty(isAnalyzing, true, nullptr);
        //    scoreState.setProperty(isVideoMuted, true, nullptr);
        //}
        //else if (curBeat == metCycleNumBeats * 2 + 1) { //now we've finished 1 listen + play cycle, so reset and do again
        //    scoreState.setProperty(isVideoPlaying, false, nullptr);
        //    scoreState.setProperty(isVideoMuted, false, nullptr);
        //    reset();
        //    curBeat = 0; //no 4-beat count in
        //}
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

void Metronome::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) 
{
    //if not scoreState pertaining to this metronome then return (although this shouldn't happen)
    if (tree != scoreState)
        return;

    if (property == scoreView) 
        setMetCycleNumBeats();
    if (property == tempo) {
        bpm = exerciseTempo[exerciseID][int(scoreState.getProperty(tempo))];
        interval = int(60.0 / bpm * sampleRate);
    }
}

// Metronome's logic for handling video and analyzing logic for if `userMode` is "Hear than Play"
void Metronome::hearThenPlayControl() 
{
    //IMPORTANT: if the beat at 1 (we've finished our 4 clicks in), we start the video to play along with it
    if (curBeat == 1) {
        scoreState.setProperty(isVideoMuted, false, nullptr);
        scoreState.setProperty(isVideoPlaying, true, nullptr);
    }
    else if (curBeat == metCycleNumBeats + 1) { //recording has just finished playing, user's turn to play 
        //this resets the video
        scoreState.setProperty(isVideoPlaying, false, nullptr);
        scoreState.setProperty(isVideoPlaying, true, nullptr);

        //Mute the video and begin analyzing
        scoreState.setProperty(isAnalyzing, true, nullptr);
        scoreState.setProperty(isVideoMuted, true, nullptr);
    }
    else if (curBeat == metCycleNumBeats * 2 + 1) { //now we've finished 1 listen + play cycle, so reset and do again
        scoreState.setProperty(isVideoMuted, false, nullptr);
        scoreState.setProperty(isVideoPlaying, false, nullptr);
        reset();
        curBeat = 0; //no 4-beat count in
    }
}

void Metronome::playControl() 
{
    if (curBeat < 0)
        scoreState.setProperty(isVideoMuted, false, nullptr);

    if (curBeat == 1) {
        DBG("inside playControl, beat 1");
        DBG("inside playControl beat == 1, videoMuted exists? " << int(scoreState.hasProperty(isVideoMuted)));
        DBG("inside playControl beat == 1, isAnalyzing exists? " << int(scoreState.hasProperty(isAnalyzing)));
        DBG("inside playControl beat == 1, isVideoPlaying exists? " << int(scoreState.hasProperty(isVideoPlaying)));

        scoreState.setProperty(isVideoMuted, true, nullptr);
        scoreState.setProperty(isAnalyzing, true, nullptr);
        scoreState.setProperty(isVideoPlaying, true, nullptr);
    }
    else if (curBeat == metCycleNumBeats) {
        scoreState.setProperty(isAnalyzing, false, nullptr);
        scoreState.setProperty(isVideoPlaying, false, nullptr);
        reset();
    }
}