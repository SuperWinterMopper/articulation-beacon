/*
  ==============================================================================

    Exercise1.h
    Created: 30 Jun 2025 11:35:20am
    Author:  afkhe

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Exercise1  : public juce::AudioAppComponent, public juce::ChangeListener
{
public:
    Exercise1();
    ~Exercise1() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;


    void paint (juce::Graphics&) override;
    void resized() override;

    //Necessary to implement due to ChangeListener being an abstract class.
    //I believe this functions effectively as a safety net for when you don't explicitly set proper state code
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;


private:
    //==============================================================================
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping,
    };

    // Function called when play button is clicked
    void playButtonClicked();
    void stopButtonClicked();

    //Function called when the state of audio is changed, (called by the immediately above functions)
    void changeState(TransportState newState);

    //Gets number of audio input/output channels for the device AB is running on
    int getNumInputChannels();
    int getNumOutputChannels();

    //Hooks up audio file to the transportSource
    void setUpAudioFile();

    //Sets up logger at the application Data Directory (Appdata/Roaming/ for Windows, Library/Application Support/ for Mac
    void setUpLogger();

    juce::TextButton playButton, stopButton;
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;

    std::unique_ptr<juce::FileLogger> fileLogger;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Exercise1)
};
