/*
  ==============================================================================

    ExerciseSelector.cpp
    Created: 30 Jun 2025 8:15:56pm
    Author:  afkhe

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ExerciseSelector.h"

//==============================================================================
ExerciseSelector::ExerciseSelector()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    addAndMakeVisible(exercise1Button);
    exercise1Button.setButtonText("Exercise 1");
    addAndMakeVisible(exercise2Button);
    exercise2Button.setButtonText("Exercise 2");
    addAndMakeVisible(exercise3Button);
    exercise3Button.setButtonText("Exercise 3");
    addAndMakeVisible(exercise4Button);
    exercise4Button.setButtonText("Exercise 4");
    setSize(420, 500);
}

ExerciseSelector::~ExerciseSelector()
{
}

void ExerciseSelector::paint (juce::Graphics& g)
{
    g.setColour (juce::Colours::grey);
}

void ExerciseSelector::resized()
{
    int buttonHeight = 50;
    int spacing = 20;
    int y = 0;

    exercise1Button.setBounds(0, y, getWidth(), buttonHeight);
    y += buttonHeight + spacing;
    exercise2Button.setBounds(0, y, getWidth(), buttonHeight);
    y += buttonHeight + spacing;
    exercise3Button.setBounds(0, y, getWidth(), buttonHeight);
    y += buttonHeight + spacing;
    exercise4Button.setBounds(0, y, getWidth(), buttonHeight);
    y += buttonHeight + spacing;
}
