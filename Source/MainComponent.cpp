#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    addAndMakeVisible(exerciseSelector);
    
    appTitle.setFont(juce::Font(70.0f, juce::Font::bold));
    appTitle.setText("ARTICULATION BEACON", juce::dontSendNotification);
    //appTitle.setColour(juce::Label::textColourId, juce::Colours::yellow);
    appTitle.setColour(juce::Label::textColourId, juce::Colour(0xFF30cdca));
    appTitle.setJustificationType(juce::Justification::horizontallyCentred);
    addAndMakeVisible(appTitle);

    setLookAndFeel(&ABLook);

    setSize(1280, 720);
}

MainComponent::~MainComponent()
{
    setLookAndFeel(nullptr);
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

    //appTitle.setBounds(15, 15, 500, 500);
    appTitle.setBounds(titleXPadding, titleYPadding, appTitle.getFont().getStringWidth(appTitle.getText()), appTitle.getFont().getHeight());
    exerciseSelector.setBounds(exerciseSelectXPadding, exerciseSelectYPadding, getWidth() - 2 * exerciseSelectXPadding, getHeight());

    DBG("the appTitle's bounds are " << appTitle.getBounds().toString());
    appTitle.toFront(false);

}