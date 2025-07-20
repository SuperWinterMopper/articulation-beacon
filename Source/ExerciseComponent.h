#pragma once

#include <JuceHeader.h>
#include "Navbar.h"
#include "VideoPlayer.h"
#include "Constants.h"

//==============================================================================
class ExerciseComponent : public juce::AudioAppComponent, private juce::ValueTree::Listener
{
public:
    ExerciseComponent(int exerciseID, ViewOptions thisComponentView, juce::ValueTree a_viewState);
    ~ExerciseComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;


    //this function is called by the home button to get back to home screen
    std::function<void()> homeButtonClick;

private:
    int exerciseID;
    ViewOptions thisComponentView;
    VideoPlayer videoPlayer;
    Navbar navBar;

    //This viewState owned by ExerciseComponent points to the viewState owned by MainComponent.
    //In other words, the viewState here reflects the current view state of the app
    juce::ValueTree curView;

    //scoreState is a crucial ValueTree object that keeps track of the state of the application's metronome, video, DSP, etc
    juce::ValueTree scoreState;

    void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;

    void configScoreState();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExerciseComponent)
};
