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
            exercisesArray[0].setVisible(false);
            break;
        case ViewOptions::EX2:
            exercisesArray[1].setVisible(false);
            break;
        case ViewOptions::EX3:
            exercisesArray[2].setVisible(false);
            break;
        case ViewOptions::EX4:
            exercisesArray[3].setVisible(false);
            break;
    }

    //set the newView to visible, completing the view switch
    switch (newView) {
        case ViewOptions::HOME:
            appTitle.setVisible(true);
            exerciseSelector.setVisible(true);
            break;
        case ViewOptions::EX1:
            exercisesArray[0].setBounds(getLocalBounds());
            exercisesArray[0].setVisible(true);
            break;
        case ViewOptions::EX2:
            exercisesArray[1].setBounds(getLocalBounds());
            exercisesArray[1].setVisible(true);
            break;
        case ViewOptions::EX3:
            exercisesArray[2].setBounds(getLocalBounds());
            exercisesArray[2].setVisible(true);
            break;
        case ViewOptions::EX4:
            exercisesArray[3].setBounds(getLocalBounds());
            exercisesArray[3].setVisible(true);
            break;
    }

    //set the current view officially to newView
    curView = newView;

    resized(); // keep layout right  
}

void MainComponent::setUpExerciseComponents() {

    for (ExerciseComponent& exComp : exercisesArray)
    {
        //connect homeButtonClick to viewSwitch. not the greatest programming but for now it works
        exComp.homeButtonClick = [this]() { viewSwitch(ViewOptions::HOME); };
        addChildComponent(exComp);
    }
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