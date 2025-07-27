#pragma once

#include <JuceHeader.h>
#include "utils.h"

//==============================================================================
/*
*/
class VideoPlayer  : public juce::Component, private juce::ValueTree::Listener
{
public:
    VideoPlayer(juce::ValueTree a_scoreState);
    ~VideoPlayer() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void setVideoPathAndLoad(juce::String path);

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property);

private:
    juce::ValueTree scoreState;
    juce::File filePath;
    juce::VideoComponent video{true};


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoPlayer)
};
