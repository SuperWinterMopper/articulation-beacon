#include <JuceHeader.h>
#include "VideoPlayer.h"

//==============================================================================
VideoPlayer::VideoPlayer() 
{
    addAndMakeVisible(video);
    DBG("addAndMakeVisible(video); HAS BEEN CALLED");
    setSize(1024, 768);
}

VideoPlayer::~VideoPlayer()
{
    
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
    path = "Resources/Videos/Exercise_2/line_1.mp4";
    juce::File newFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile(path);
    DBG("isShowing is " << (isShowing() ? "true" : "false"));

    if (isShowing() && newFile.existsAsFile()) {
        filePath = newFile;
        DBG("LOADING VIDEO FILE" << filePath.getFullPathName());

        auto r = video.load(filePath);
        if (!r.wasOk())
            DBG("Video load failed: " << r.getErrorMessage());
        else
            video.play();
    }
    else {
        juce::String error_message = "Error when attempting to open video file " + newFile.getFullPathName();
        juce::Logger::writeToLog(error_message);
        DBG(error_message);
    }

}