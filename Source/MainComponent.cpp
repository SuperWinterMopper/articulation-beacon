#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    setSize(1280, 720);
    addAndMakeVisible(exerciseSelector);

    setLookAndFeel(&ABLook);
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
    int exerciseSelectXPadding = 20;
    int exerciseSelectYPadding = 300;

    exerciseSelector.setBounds(exerciseSelectXPadding, exerciseSelectYPadding, getWidth() - 2 * exerciseSelectXPadding, getHeight());
}