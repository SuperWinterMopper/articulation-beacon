#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class VideoPlayer  : public juce::Component
{
public:
    VideoPlayer(juce::String file);
    ~VideoPlayer() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void setFileLocation(juce::String path);

private:
    juce::File file_location;
    juce::VideoComponent video(true);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoPlayer)
};
