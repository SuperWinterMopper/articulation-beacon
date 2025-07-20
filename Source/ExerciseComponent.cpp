#include <JuceHeader.h>
#include "ExerciseComponent.h"

//==============================================================================
ExerciseComponent::ExerciseComponent(int exerciseID, ViewOptions thisComponentView, juce::ValueTree a_viewState)
    : exerciseID(exerciseID), thisComponentView(thisComponentView), curView(a_viewState)
{
    configScoreState();
    configInputOutput();

    curView.addListener(this);
    scoreState.addListener(this);

    //asks MainComponent to update go to home
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

void ExerciseComponent::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) {
    DBG("valueTreePropertyChanged in ExerciseComponent CALLED!!!");
    //returns if state changed for another component, not this one
    if (thisComponentView != static_cast<ViewOptions>((int)tree.getProperty(property))) return;

    if (property == viewState) {
        juce::String videosPath = "Resources/Videos/Exercise_2/line_1.mp4";
        videoPlayer.setVideoPathAndLoad(videosPath);
    }
    else if (property == scoreView) {

    }
    else if (property == isVideoPlaying) {

    }
    else if (property == isAnalyzing) {

    }
    else if (property == tempo) {

    }
    else {
        DBG("This property doesn't exist...");
    }
}

void ExerciseComponent::resized()
{
    const int navBarHeight = 100;
    const int videoPaddingX = 100, videoPaddingY = 20;
    const int videoWidth = getWidth() - 2 * videoPaddingX, videoHeight = navBarHeight * 2;

    navBar.setBounds(0, getHeight() - navBarHeight, getWidth(), navBarHeight);

    videoPlayer.setBounds(videoPaddingX, getHeight() - navBarHeight - videoHeight - videoPaddingY, videoWidth, videoHeight);
}

void ExerciseComponent::configScoreState() {
    scoreState.setProperty(scoreView, 0, nullptr);
    scoreState.setProperty(isVideoPlaying, false, nullptr);
    scoreState.setProperty(isAnalyzing, false, nullptr);
    scoreState.setProperty(tempo, 90, nullptr); //set to 90bpm, temporary. In future set to actual value based on other file
}

void ExerciseComponent::configInputOutput() {

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [&](bool granted) { setAudioChannels(granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels(2, 2);
    }
}