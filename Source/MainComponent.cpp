#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    appTitle.setFont(juce::Font(70.0f, juce::Font::bold));
    appTitle.setText("ARTICULATION BEACON", juce::dontSendNotification);
    appTitle.setColour(juce::Label::textColourId, juce::Colour(0xFF30cdca));
    appTitle.setJustificationType(juce::Justification::horizontallyCentred);
    addAndMakeVisible(appTitle);

    exerciseSelector.onSelectExercise = [this] (int i) {viewOpener(i); };
    addAndMakeVisible(exerciseSelector);

    //attach the ABLook visual style
    setLookAndFeel(&ABLook);

    // Set up file logging
    setUpLogger();

    setSize(1280, 720);
}

MainComponent::~MainComponent()
{
    setLookAndFeel(nullptr);
    juce::Logger::setCurrentLogger(nullptr);
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    int titleXPadding = (getWidth() / 2) - appTitle.getFont().getStringWidth(appTitle.getText()) / 2;
    int titleYPadding = 50;

    int exerciseSelectXPadding = 200;
    int exerciseSelectYPadding = 300;

    appTitle.setBounds(titleXPadding, titleYPadding, appTitle.getFont().getStringWidth(appTitle.getText()), appTitle.getFont().getHeight());
    exerciseSelector.setBounds(exerciseSelectXPadding, exerciseSelectYPadding, getWidth() - 2 * exerciseSelectXPadding, getHeight());

    appTitle.toFront(false);
}

void MainComponent::viewOpener(int viewNum) {

    // viewNum == 0 corresponds to back to home screen button
    if (viewNum == 0) {
        appTitle.setVisible(true);
        exerciseSelector.setVisible(true);
    }
    //otherwise we're selecting an exercise
    else {
        appTitle.setVisible(false);
        exerciseSelector.setVisible(false);

        switch (viewNum) {
            case 1:
                exercise1Page.setBounds(getLocalBounds());
                addAndMakeVisible(exercise1Page);
                break;
            case 2:
                exercise2Page.setBounds(getLocalBounds());
                addAndMakeVisible(exercise2Page);
                break;
            case 3:
                addAndMakeVisible(exercise3Page);
                break;
            case 4:
                addAndMakeVisible(exercise4Page);
                break;
        }
    }
}

void MainComponent::setUpLogger()
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