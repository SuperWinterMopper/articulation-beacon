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
    // IMPORTANT: Declare state before components that depend on it
    juce::ValueTree scoreState;   // score/metronome/video/DSP state
    juce::ValueTree curView;      // reflects app view state (shared from MainComponent)

    int exerciseID = 0; // 0 for safety
    ViewOptions thisComponentView;

    // Dependent components (constructed in .cpp initializer list)
    VideoPlayer videoPlayer;  
    Navbar navBar;            
    Metronome metronome;     
    Graph graph;             

    void valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) override;

    void configScoreState();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExerciseComponent)
};
