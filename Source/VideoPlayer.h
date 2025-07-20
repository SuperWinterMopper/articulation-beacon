#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class VideoPlayer  : public juce::Component
{
public:
    VideoPlayer();
    ~VideoPlayer() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void setVideoPathAndLoad(juce::String path);

private:
    juce::File filePath;
    juce::VideoComponent video{true};
    juce::ValueTree viewState;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoPlayer)
};
