#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    //attach, but not display exercise components
    setUpExerciseComponents();

    appTitle.setFont(juce::Font(70.0f, juce::Font::bold));
    appTitle.setText("ARTICULATION BEACON", juce::dontSendNotification);
    appTitle.setColour(juce::Label::textColourId, juce::Colour(0xFF30cdca));
    appTitle.setJustificationType(juce::Justification::horizontallyCentred);
    addAndMakeVisible(appTitle);

    //add exerciseSelector and set connect it's button switch to viewSwitch function in MainComponent
    exerciseSelector.onSelectExercise = [this] (ViewOptions newView) { viewSwitch(newView); };
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
    exercise1Page.setBounds(getLocalBounds());
    exercise2Page.setBounds(getLocalBounds());
    exercise3Page.setBounds(getLocalBounds());
    exercise4Page.setBounds(getLocalBounds());
}

//Handles logic for switching views. Note that viewSwitch assumes all components in ViewOptions have already been attached to MainComponent
void MainComponent::viewSwitch(ViewOptions newView) {
    if (newView == curView) return;

    //set the curView to invisible to switch to the newView
    switch (curView) {
        case ViewOptions::HOME:
            appTitle.setVisible(false);
            exerciseSelector.setVisible(false);
            break;
        case ViewOptions::EX1:
            exercise1Page.setVisible(false);
            break;
        case ViewOptions::EX2:
            exercise2Page.setVisible(false);
            break;
        case ViewOptions::EX3:
            exercise3Page.setVisible(false);
            break;
        case ViewOptions::EX4:
            exercise4Page.setVisible(false);
            break;
    }

    //set the newView to visible, completing the view switch
    switch (newView) {
        case ViewOptions::HOME:
            appTitle.setVisible(true);
            exerciseSelector.setVisible(true);
            break;
        case ViewOptions::EX1:
            exercise1Page.setBounds(getLocalBounds());
            exercise1Page.setVisible(true);
            break;
        case ViewOptions::EX2:
            exercise2Page.setBounds(getLocalBounds());
            exercise2Page.setVisible(true);
            break;
        case ViewOptions::EX3:
            exercise3Page.setBounds(getLocalBounds());
            exercise3Page.setVisible(true);
            break;
        case ViewOptions::EX4:
            exercise4Page.setBounds(getLocalBounds());
            exercise4Page.setVisible(true);
            break;
    }

    //set the current view officially to newView
    curView = newView;

    resized(); // keep layout right  
}

void MainComponent::setUpExerciseComponents() {

    //this is called bad programming but it works and I can't be bothered for the scope of this project
    exercise2Page.homeButtonClick = [this]() { viewSwitch(ViewOptions::HOME); };

    addChildComponent(exercise1Page);
    addChildComponent(exercise2Page);
    addChildComponent(exercise3Page);
    addChildComponent(exercise4Page);
}

void MainComponent::setUpLogger() {
    auto logFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory).getChildFile("Articulation Beacon").getChildFile("Articulation Beacon.log");

    // Create directory if it doesn't exist
    logFile.getParentDirectory().createDirectory();

    fileLogger = std::make_unique<juce::FileLogger>(logFile, "Articulation Beacon started");
    juce::Logger::setCurrentLogger(fileLogger.get());

    if (juce::Logger::getCurrentLogger() != nullptr)
        juce::Logger::writeToLog("App initialization log.");
}