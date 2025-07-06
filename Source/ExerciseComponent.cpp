#include <JuceHeader.h>
#include "ExerciseComponent.h"

//==============================================================================
ExerciseComponent::ExerciseComponent()
{
    //asks MainComponent to update go to home
    navBar.homeButtonClick = [this]() { if (homeButtonClick) homeButtonClick(); };
    addAndMakeVisible(navBar);
}

ExerciseComponent::~ExerciseComponent()
{
}

void ExerciseComponent::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("ExerciseComponent", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void ExerciseComponent::resized()
{
    const int navBarHeight = 100;

    navBar.setBounds(0, getHeight() - navBarHeight, getWidth(), navBarHeight);
}
