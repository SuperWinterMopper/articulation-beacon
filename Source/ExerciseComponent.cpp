#include <JuceHeader.h>
#include "ExerciseComponent.h"

//==============================================================================
ExerciseComponent::ExerciseComponent(int exerciseID, ViewOptions thisComponentView, juce::ValueTree a_viewState)
    : exerciseID(exerciseID), thisComponentView(thisComponentView), curView(a_viewState)
{
    curView.addListener(this);
    scoreState.addListener(this);

    int setbpm = exerciseTempo[exerciseID][(int)scoreState.getProperty(tempo)];
    DBG("setbpm is getting set to " << exerciseTempo[exerciseID][(int)scoreState.getProperty(tempo)] << " for this exercise");
    metronome.setBPM(exerciseTempo[exerciseID][(int)scoreState.getProperty(tempo)]);

    //asks MainComponent to update go to home
    navBar.homeButtonClick = [this]() { if (homeButtonClick) homeButtonClick(); };
    addAndMakeVisible(videoPlayer);
    addAndMakeVisible(navBar);
}

ExerciseComponent::~ExerciseComponent()
{
    shutdownAudio();
}

void ExerciseComponent::paint (juce::Graphics& g) {    
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}

void ExerciseComponent::resized()
{
    const int navBarHeight = 100;
    const int videoPaddingX = 100, videoPaddingY = 20;
    const int videoWidth = getWidth() - 2 * videoPaddingX, videoHeight = navBarHeight * 6;

    navBar.setBounds(0, getHeight() - navBarHeight, getWidth(), navBarHeight);

    videoPlayer.setBounds(videoPaddingX, getHeight() - navBarHeight - videoHeight - videoPaddingY, videoWidth, videoHeight);
}

void ExerciseComponent::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) {
    if (property == viewState) { //Operates if this exercise may be selected, or ANY exercise may be deselected. Note this also implies the tree here is curView
        //we are turning this on
        if (thisComponentView == static_cast<ViewOptions>((int)tree.getProperty(viewState))) {
            DBG("INSIDE valueTreePropertyChanged of ExerciseComponet, about to call configScoreState");
            configScoreState();
            configInputOutput();

            juce::String exerciseNum = juce::String(static_cast<int>(thisComponentView));
            juce::String videosPath = "Resources/Videos/ex" + exerciseNum + "WholeMod.mp4";
            videoPlayer.setVideoPathAndLoad(videosPath);
        }
        else {
            videoPlayer.stopVideo();
            metronome.reset();
            shutdownAudio();
        }
    }
    
    //return if the tree is not this specific component's scoreState
    if (tree != scoreState) 
        return;

    else if (property == userMode) {
        //the logic for the 2 user modes's logics are handled in Metronome
        scoreState.setProperty(isVideoPlaying, false, nullptr);
        scoreState.setProperty(isAnalyzing, false, nullptr);
        scoreState.setProperty(isMetronomePlaying, false, nullptr);
        videoPlayer.stopVideo();
        metronome.reset();
    }
    else if (property == scoreView) {

    }
    else if (property == isVideoPlaying) { //this is switched when Navbar play button is pressed
        
    }
    else if (property == isAnalyzing) {

    }
    else if (property == tempo) {
        scoreState.setProperty(isVideoPlaying, false, nullptr);
        scoreState.setProperty(isAnalyzing, false, nullptr);
        scoreState.setProperty(isMetronomePlaying, false, nullptr);
        videoPlayer.stopVideo();
        metronome.reset();
        
        bool fast = int(scoreState.getProperty(tempo)) == 0 ? false : true;

        juce::String videosPath = "Resources/Videos/ex" + std::to_string(exerciseID + 1) + "WholeMod" + (fast ? "F" : "") + ".mp4";
        videoPlayer.setVideoPathAndLoad(videosPath);
    }
    else if (property == isMetronomePlaying) {
        metronome.reset();
    }
    else {
        DBG("This property doesn't exist...");
    }
}

void ExerciseComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    metronome.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void ExerciseComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    //keep operating metronome if we're supposed to
    if ( (bool) scoreState.getProperty(isMetronomePlaying) == true)
    {
        metronome.getNextAudioBlock(bufferToFill);
    }
    else //stop metronome
    {
        metronome.reset();
        scoreState.setProperty(isVideoPlaying, false, nullptr);
    }
}

void ExerciseComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}


void ExerciseComponent::configScoreState() {
    if (exerciseID < 0 || exerciseID >= exerciseTempo.size()) {
        DBG("exerciseID is not correct, it's " << exerciseID);
        jassert(false);
    }

    scoreState.setProperty(scoreView, "Whole", nullptr);
    scoreState.setProperty(userMode, "Hear then Play", nullptr);
    scoreState.setProperty(isVideoPlaying, false, nullptr);
    scoreState.setProperty(isAnalyzing, false, nullptr);
    scoreState.setProperty(tempo, 0, nullptr); //starts at slow tempo by default
    scoreState.setProperty(isMetronomePlaying, false, nullptr);
    scoreState.setProperty(isVideoMuted, false, nullptr);
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
