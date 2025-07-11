#pragma once

#include <JuceHeader.h>
#include "Navbar.h"
#include "VideoPlayer.h"

//==============================================================================
class ExerciseComponent  : public juce::Component
{
public:
    ExerciseComponent(juce::String videosPath);
    ~ExerciseComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    //this function is called by the home button to get back to home screen
    std::function<void()> homeButtonClick;

private:
    juce::String videosPath;
    VideoPlayer videoPlayer;
    Navbar navBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExerciseComponent)
};
