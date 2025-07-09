#include <JuceHeader.h>
#include "VideoPlayer.h"

//==============================================================================
VideoPlayer::VideoPlayer(juce::String path) 
{
    setFileLocation(path);
    
    if (auto r = video.load(file_location); r.wasOk())
        video.play();
    else {
        juce::String error_message = "Video load failed for file at location " + file_location.getFullPathName() + ", " + r.getErrorMessage();
        juce::Logger::writeToLog(error_message);
        DBG(error_message);
    }

    addAndMakeVisible(video);
    setSize(640, 360);
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

void VideoPlayer::setFileLocation(juce::String path) 
{
    file_location = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile(path);
}