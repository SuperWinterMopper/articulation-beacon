#include <JuceHeader.h>
#include "ExerciseComponent.h"

//==============================================================================
ExerciseComponent::ExerciseComponent(juce::String videosPath) : videosPath(videosPath), videoPlayer{ videosPath }
                                                                
{
    //asks MainComponent to update go to home
    DBG("the videoPath for this ExerciseComponent right before videoPlayer.setFileLocation(videosPath); is " << videosPath);
    videoPlayer.setFileLocation(videosPath);
    navBar.homeButtonClick = [this]() { if (homeButtonClick) homeButtonClick(); };
    addAndMakeVisible(videoPlayer);
    addAndMakeVisible(navBar);
}

ExerciseComponent::~ExerciseComponent()
{

}

void ExerciseComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}

void ExerciseComponent::resized()
{
    const int navBarHeight = 100;

    navBar.setBounds(0, getHeight() - navBarHeight, getWidth(), navBarHeight);
}
