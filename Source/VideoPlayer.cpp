#include <JuceHeader.h>
#include "VideoPlayer.h"

//==============================================================================
VideoPlayer::VideoPlayer(juce::String path) 
{
    setFileLocation(path);
    addAndMakeVisible(video);
    DBG("addAndMakeVisible(video); HAS BEEN CALLED");
    setSize(640, 360);
    
    //DBG("the file location is " << file_location.getFullPathName());

    //if (file_location.existsAsFile()) {
    //    DBG("the file location right before error is " << file_location.getFullPathName());
    //    auto r = video.load(file_location);
        //if (r.wasOk()) {
        //    video.play();
        //}
        //else {
        //    juce::String error_message = "Video load failed for file at location " + file_location.getFullPathName() + ", " + r.getErrorMessage();
        //    juce::Logger::writeToLog(error_message);
        //    DBG(error_message);
        //}
    //}
    //else {
    //    juce::String error_message = "Video file does not exist at location " + file_location.getFullPathName();
    //    juce::Logger::writeToLog(error_message);
    //    DBG(error_message);
    //}

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
    juce::File newFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile(path);
    if(newFile.existsAsFile())
        file_location = newFile;
    else {
        juce::String error_message = "Error when attempting to open video file " + newFile.getFullPathName();
        juce::Logger::writeToLog(error_message);
        DBG(error_message);
    }
}

void VideoPlayer::parentHierarchyChanged() {
    Component::parentHierarchyChanged();
    DBG("CALLED parentHierarchyChanged");

    if (isShowing() && file_location.existsAsFile()) {
        loadVideoFile();
    }
}

//note this function assumes path is a valid file path to a video file
void VideoPlayer::loadVideoFile() {
    DBG("CALLED loadVideoFile");
    auto r = video.load(file_location);
    //if (r.wasOk()) {
    //    video.play();
    //}
    //else {
    //    juce::String error_message = "Video load failed: " + r.getErrorMessage();
    //    juce::Logger::writeToLog(error_message);
    //    DBG(error_message);
    //}
}