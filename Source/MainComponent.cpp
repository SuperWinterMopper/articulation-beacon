#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    curView.setProperty(viewState, static_cast<int>(ViewOptions::HOME), nullptr);

    //attach, but not display exercise components
    setUpExerciseComponents();

    initializeCOM();

    appTitle.setFont(juce::Font(70.0f, juce::Font::bold));
    appTitle.setText("ARTICULATION MONKEY", juce::dontSendNotification);
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

    configInputOutput();

    setBounds(juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea);
}

MainComponent::~MainComponent()
{
    setLookAndFeel(nullptr);
    shutdownAudio();
    juce::Logger::setCurrentLogger(nullptr);
    #if JUCE_WINDOWS
        if (comInitialized)
        {
            CoUninitialize();
            comInitialized = false;
        }
    #endif
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
    int exerciseSelectYPadding = 200;

    appTitle.setBounds(titleXPadding, titleYPadding, appTitle.getFont().getStringWidth(appTitle.getText()), appTitle.getFont().getHeight());
    exerciseSelector.setBounds(exerciseSelectXPadding, exerciseSelectYPadding, getWidth() - 2 * exerciseSelectXPadding, getHeight());
    appTitle.toFront(false);

    for (int i = 0; i < NUM_EXERCISES; i++) 
        exercisesArray[i].setBounds(getLocalBounds());
}

//Handles logic for switching views. Note that viewSwitch assumes all components in ViewOptions have already been attached to MainComponent
void MainComponent::viewSwitch(ViewOptions newViewOption) {
    DBG("CALLED viewSwitch");
    ViewOptions curViewOption = static_cast<ViewOptions>((int)curView.getProperty(viewState));

    if (curViewOption == newViewOption) return;

    //========================================================
    //set the curView to invisible to switch to the newView
    if (curViewOption == ViewOptions::HOME) {
        appTitle.setVisible(false);
        exerciseSelector.setVisible(false);
    }
    else {
        int i = static_cast<int>(curViewOption) - static_cast<int>(ViewOptions::EX1); 
        exercisesArray[i].setVisible(false);
    }
    //========================================================

    //========================================================
    //set the newView to visible, completing the view switch
    if (newViewOption == ViewOptions::HOME) {
        currentExerciseIndex = -1;
        appTitle.setVisible(true);
        exerciseSelector.setVisible(true);
    }
    else {
        currentExerciseIndex = static_cast<int>(newViewOption) - static_cast<int>(ViewOptions::EX1);
        exercisesArray[currentExerciseIndex].setBounds(getLocalBounds());
        exercisesArray[currentExerciseIndex].setVisible(true);
    }
    //========================================================

    //set the current view officially to newView
    DBG("setting valueTree curView to a new value, " << static_cast<int>(newViewOption));
    curView.setProperty(viewState, static_cast<int>(newViewOption), nullptr);

    resized(); // keep layout right  
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // let every exercise set up its metronome, etc.
    for (auto& ex : exercisesArray)
        ex.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (currentExerciseIndex >= 0) {
        exercisesArray[currentExerciseIndex].getNextAudioBlock(bufferToFill);
    }
}

void MainComponent::releaseResources()
{
    for (auto& ex : exercisesArray)
        ex.releaseResources();
}


void MainComponent::setUpExerciseComponents() {
    for (ExerciseComponent& exComp : exercisesArray)
    {
        //connect homeButtonClick to viewSwitch. not the greatest programming but for now it works
        exComp.homeButtonClick = [this]() { viewSwitch(ViewOptions::HOME); };
        addChildComponent(exComp);
    }
}

void MainComponent::configInputOutput() {

    int inputChannels = 2, outputChannels = 2;

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [&](bool granted) { setAudioChannels(granted ? inputChannels : 0, outputChannels); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels(inputChannels, outputChannels);
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

void MainComponent::initializeCOM() {
    #if JUCE_WINDOWS
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
        if (SUCCEEDED(hr))
        {
            comInitialized = true;
        }
        else
        {
            // Handle the error appropriately
            juce::Logger::writeToLog("Failed to initialize COM");
        }
    #endif
}