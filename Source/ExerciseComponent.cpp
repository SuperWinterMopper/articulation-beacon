#include <JuceHeader.h>
#include "ExerciseComponent.h"

//==============================================================================
ExerciseComponent::ExerciseComponent(int exerciseID, ViewOptions thisComponentView, juce::ValueTree a_viewState)
    : exerciseID(exerciseID), thisComponentView(thisComponentView), curView(a_viewState)
{
    curView.addListener(this);

    //asks MainComponent to update go to home
    navBar.homeButtonClick = [this]() { if (homeButtonClick) homeButtonClick(); };
    addAndMakeVisible(videoPlayer);
    addAndMakeVisible(navBar);
}

ExerciseComponent::~ExerciseComponent()
{

}

void ExerciseComponent::paint (juce::Graphics& g)
{    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

}

void ExerciseComponent::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) {
    DBG("valueTreePropertyChanged in ExerciseComponent CALLED!!!");

    //returns if state changed for another component, not this one
    if (thisComponentView != static_cast<ViewOptions>((int)treeWhosePropertyHasChanged.getProperty(property))) return;

    //TEMPORARY FOR TESTING. Every component will determine the files it must open based on it's exerciseID and Constants.h in the future
    juce::String videosPath = "Resources/Videos/Exercise_2/line_1.mp4";
    videoPlayer.setVideoPathAndLoad(videosPath);
}


void ExerciseComponent::resized()
{
    const int navBarHeight = 100;

    navBar.setBounds(0, getHeight() - navBarHeight, getWidth(), navBarHeight);
}
