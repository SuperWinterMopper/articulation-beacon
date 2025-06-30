/*
  ==============================================================================

    Exercise1.cpp
    Created: 30 Jun 2025 11:35:20am
    Author:  afkhe

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Exercise1.h"

//==============================================================================
Exercise1::Exercise1() : state(Stopped)
{
    // Initialize buttons 
    //==============================================================================
    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play audio");
    playButton.onClick = [this] { playButtonClicked(); };
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    playButton.setEnabled(true);

    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop audio");
    stopButton.onClick = [this] { stopButtonClicked(); };
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    stopButton.setEnabled(false);
    //==============================================================================

    formatManager.registerBasicFormats(); // register the standard audio formats
    transportSource.addChangeListener(this);

    // Make sure you set the size of the component after
    // you add any child components.
    setSize(800, 600);


    //Set audio channels
    //==============================================================================
    //int numOutputChannels = getNumOutputChannels();
    int numOutputChannels = 2;
    int numInputChannels = 0; //we're just trying output for now

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [&](bool granted) { setAudioChannels(granted ? numInputChannels : 0, numOutputChannels); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        // For now set to stereo output

        setAudioChannels(numInputChannels, numOutputChannels);
    }

    //this will hook up transportSource to the right audio file
    setUpAudioFile();

    // Set up file logging
    setUpLogger();
}

Exercise1::~Exercise1()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
    juce::Logger::setCurrentLogger(nullptr);

}

//==============================================================================
void Exercise1::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.
    DBG("Sample rate: " << sampleRate << ", Samples per block: " << samplesPerBlockExpected);

    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void Exercise1::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
    {
        DBG("readerSource is nullptr!");
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    transportSource.getNextAudioBlock(bufferToFill);


    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    //bufferToFill.clearActiveBufferRegion();
}

void Exercise1::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
    transportSource.releaseResources();
}

void Exercise1::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void Exercise1::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    playButton.setBounds(10, 10, getWidth() - 20, 20);
    stopButton.setBounds(10, 70, getWidth() - 20, 20);
}

void Exercise1::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
        //this aligns the `state` variable to what's actually true, if it's not
        if (transportSource.isPlaying())
            changeState(Playing);
        else
            changeState(Stopped);
    }
}

void Exercise1::playButtonClicked()
{
    changeState(Starting);
}

void Exercise1::stopButtonClicked()
{
    changeState(Stopping);
}

void Exercise1::changeState(TransportState newState)
{
    if (newState != state) {
        DBG("changeState called!!!!!");
        juce::Logger::writeToLog("changeState called!!!!");

        state = newState;

        switch (state) {
        case Stopped: //switches to this state when stopping has finished
            playButton.setEnabled(true);
            transportSource.setPosition(0.0); //set to start playing at beginning if start again
            break;

        case Starting:
            playButton.setEnabled(false);
            transportSource.start();
            break;

        case Playing:
            stopButton.setEnabled(true);
            break;

        case Stopping:
            stopButton.setEnabled(false);
            transportSource.stop();
            break;
        }
    }
}

int Exercise1::getNumOutputChannels() {
    int numOutputChannels = 0; //returns 0 by default
    auto device = deviceManager.getCurrentAudioDevice();

    if (device != nullptr)
    {
        numOutputChannels = device->getActiveOutputChannels().countNumberOfSetBits();
    }
    return numOutputChannels;
}

int Exercise1::getNumInputChannels() {
    int numInputChannels = 0; //returns 0 by default
    auto device = juce::AudioDeviceManager().getCurrentAudioDevice();

    if (device != nullptr)
    {
        numInputChannels = device->getActiveInputChannels().countNumberOfSetBits();
    }
    return numInputChannels;
}

void Exercise1::setUpAudioFile()
{
    juce::File audio_file = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("Resources/test7.wav");

    if (audio_file.existsAsFile()) {
        auto* reader = formatManager.createReaderFor(audio_file);

        if (reader != nullptr) {
            auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
            transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
            playButton.setEnabled(true);
            readerSource.reset(newSource.release());
        }
    }
    else {
        DBG("test7 is not a valid file!!");
    }
}

void Exercise1::setUpLogger()
{
    // Set up file logging
    auto logFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Articulation Beacon").getChildFile("Articulation Beacon.log");

    // Create directory if it doesn't exist
    logFile.getParentDirectory().createDirectory();

    fileLogger = std::make_unique<juce::FileLogger>(logFile, "Articulation Beacon started");
    juce::Logger::setCurrentLogger(fileLogger.get());

    if (juce::Logger::getCurrentLogger() != nullptr)
        juce::Logger::writeToLog("App initialization log.");
}