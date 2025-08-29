#pragma once

#include <JuceHeader.h>
#include "Navbar.h"
#include "VideoPlayer.h"
#include "Metronome.h"
#include "Utils.h"
#include "ScoreData.h"
#include "Graph.h"

//==============================================================================
class ExerciseComponent : public juce::Component, private juce::ValueTree::Listener
{
public:
    ExerciseComponent(int exerciseID, ViewOptions thisComponentView, juce::ValueTree a_viewState);
    ~ExerciseComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill, juce::AudioIODevice* device);
    void releaseResources();

    //this function is called by the home button to get back to home screen
    std::function<void()> homeButtonClick;

private:
    //scoreState is a crucial ValueTree object that keeps track of the state of the application's metronome, video, DSP, etc
    juce::ValueTree scoreState;

    //This viewState owned by ExerciseComponent points to the viewState owned by MainComponent.
    //In other words, the viewState here reflects the current view state of the app
    juce::ValueTree curView;

    int exerciseID = 0; //0 for safety
    ViewOptions thisComponentView;
    VideoPlayer videoPlayer{ scoreState };
    Navbar navBar{scoreState, thisComponentView};
    Metronome metronome {defaultBPM, scoreState, exerciseID };
    Graph graph;

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

    void configScoreState();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExerciseComponent)
};
