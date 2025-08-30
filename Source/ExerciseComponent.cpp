#include <JuceHeader.h>
#include "ExerciseComponent.h"

//==============================================================================
ExerciseComponent::ExerciseComponent(int exerciseID, ViewOptions thisComponentView, juce::ValueTree a_viewState)
    : exerciseID(exerciseID), thisComponentView(thisComponentView), curView(a_viewState), scoreState(scoreStateIdentifier), graph(scoreState, exerciseID)
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
    addAndMakeVisible(graph);
}

ExerciseComponent::~ExerciseComponent()
{
    videoPlayer.stopVideo();
    metronome.reset();

    // Remove listeners we added in the ctor
    curView.removeListener(this);
    scoreState.removeListener(this);
}

void ExerciseComponent::paint (juce::Graphics& g) {    
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
}

void ExerciseComponent::resized()
{
    const int navBarHeight = 100;
    const int videoPaddingX = 100, videoPaddingY = 20;
    const int videoWidth = getWidth() - 2 * videoPaddingX, videoHeight = navBarHeight * 4;

    const int graphPaddingX = videoPaddingX, graphPaddingY = 40;
    const int graphWidth = getWidth() - 2 * graphPaddingX, graphHeight = videoHeight;

    navBar.setBounds(0, getHeight() - navBarHeight, getWidth(), navBarHeight);

    videoPlayer.setBounds(videoPaddingX, getHeight() - navBarHeight - videoHeight - videoPaddingY, videoWidth, videoHeight);
    graph.setBounds(videoPaddingX, graphPaddingY, graphWidth, graphHeight);
}

void ExerciseComponent::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property) {
    if (property == viewState) { //Operates if this exercise may be selected, or ANY exercise may be deselected. Note this also implies the tree here is curView
        //we are turning this on
        if (thisComponentView == static_cast<ViewOptions>((int)tree.getProperty(viewState))) {
            DBG("INSIDE valueTreePropertyChanged of ExerciseComponet, about to call configScoreState");
            configScoreState();
            //configInputOutput();

            juce::String exerciseNum = juce::String(static_cast<int>(thisComponentView));
            juce::String videosPath = "Resources/Videos/ex" + exerciseNum + "WholeMod.mp4";
            videoPlayer.setVideoPathAndLoad(videosPath);
        }
        else {
            videoPlayer.stopVideo();
            metronome.reset();
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
        if ( (bool) scoreState.getProperty(isAnalyzing) == true) //we just started analyzing -> Graph should keep track of the time
            scoreState.setProperty(startTime, (int) juce::Time::getMillisecondCounter(), nullptr);
        else 
            scoreState.setProperty(startTime, "Unstarted", nullptr);
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
        DBG("This property: " << property.toString() << " doesn't exist... ");
    }
}

void ExerciseComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    metronome.prepareToPlay(samplesPerBlockExpected, sampleRate);
    graph.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void ExerciseComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill, juce::AudioIODevice* device)
{
    auto activeInputChannels  = device->getActiveInputChannels();
    auto activeOutputChannels = device->getActiveOutputChannels();
    auto maxInputChannels  = activeInputChannels .getHighestBit() + 1;
    auto maxOutputChannels = activeOutputChannels.getHighestBit() + 1;

    if(maxInputChannels == 0) {
        DBG("No active input channels");
        return;
    }
    
    //process input channels first
    for (auto channel = 0; channel < maxInputChannels; channel++) {
        if(activeInputChannels[channel]) {
            auto* inBuffer = bufferToFill.buffer->getReadPointer (channel, bufferToFill.startSample);
            //Perform analysis with Graph with this whole for loop
            for (auto sample = 0; sample < bufferToFill.numSamples; sample++) {
                graph.processInputSample(inBuffer[sample]);
            }
            break; //just process the first channel for now (mono)
        }
    }
    
    // Write onto bufferToFill with metronome output
    bufferToFill.clearActiveBufferRegion();

    if ( (bool) scoreState.getProperty(isMetronomePlaying) == false) //stop metronome
    {
        metronome.reset();
        scoreState.setProperty(isVideoPlaying, false, nullptr);
    }
    else { //play metronome so fill the buffer with its data
        metronome.getNextAudioBlock(bufferToFill, 0);
    }
}

void ExerciseComponent::releaseResources()
{
    metronome.reset();
    graph.releaseResources();
}

void ExerciseComponent::configScoreState() {
    if (exerciseID < 0 || exerciseID >= exerciseTempo.size()) {
        DBG("exerciseID is not correct, it's " << exerciseID);
        jassert(false);
    }

    scoreState.setProperty(scoreView, "Whole", nullptr);
    scoreState.setProperty(userMode, "Hear then Play", nullptr);
    scoreState.setProperty(isAnalyzing, false, nullptr);
    scoreState.setProperty(tempo, 0, nullptr); //starts at slow tempo by default
    scoreState.setProperty(isMetronomePlaying, false, nullptr);
    scoreState.setProperty(isVideoMuted, false, nullptr);
    scoreState.setProperty(isVideoPlaying, false, nullptr);
    scoreState.setProperty(startTime, "Unstarted", nullptr);
}

//void ExerciseComponent::configInputOutput() {
//
//    int inputChannels = 0, outputChannels = 2;
//
//    // Some platforms require permissions to open input channels so request that here
//    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
//        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
//    {
//        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
//            [&](bool granted) { setAudioChannels(granted ? inputChannels : 0, outputChannels); });
//    }
//    else
//    {
//        // Specify the number of input and output channels that we want to open
//        setAudioChannels(inputChannels, outputChannels);
//    }
//}
