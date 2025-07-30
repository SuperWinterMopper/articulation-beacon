#include <JuceHeader.h>
#include "VideoPlayer.h"

//==============================================================================
VideoPlayer::VideoPlayer(juce::ValueTree a_scoreState) : scoreState(a_scoreState)
{
    scoreState.addListener(this);

    DBG("addAndMakeVisible(video); HAS BEEN CALLED");
    setSize(1024, 768);
    addAndMakeVisible(video);
}

VideoPlayer::~VideoPlayer()
{
    
}

void VideoPlayer::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    if (property == isVideoPlaying) {
        //if we're starting to play video
        if ((bool)tree.getProperty(isVideoPlaying) == true) {
            video.setAudioVolume(defaultAudioLevel);
            video.play();
        }
        else {
            video.stop();
            video.setPlayPosition(0.0);
        }
    }
    if (property == isVideoMuted) {
        //the changed property is whether it's muted, if it is then set audio level to 0.0
        if ((bool)tree.getProperty(isVideoMuted) == true) {
            video.setAudioVolume(0.0);
        }
        else {
            video.setAudioVolume(defaultAudioLevel);
        }
    }

    //add more state updates if needed
}

void VideoPlayer::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("VideoPlayer", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void VideoPlayer::resized()
{
    video.setBounds(getLocalBounds());
}

void VideoPlayer::setVideoPathAndLoad(juce::String path)
{
    DBG("WILL EXECUTE VIDEO LOAD CODE");
    juce::File newFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile(path);
    DBG("isShowing is " << (isShowing() ? "true" : "false"));

    if (isShowing() && newFile.existsAsFile()) {
        filePath = newFile;
        DBG("LOADING VIDEO FILE" << filePath.getFullPathName());

        auto r = video.load(filePath);
        if (!r.wasOk()) DBG("Video load failed: " << r.getErrorMessage());
    }
    else {
        juce::String error_message = "Error when attempting to open video file " + newFile.getFullPathName();
        juce::Logger::writeToLog(error_message);
        DBG(error_message);
    }
}