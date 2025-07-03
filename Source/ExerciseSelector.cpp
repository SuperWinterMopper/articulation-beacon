#include <JuceHeader.h>
#include "ExerciseSelector.h"

//==============================================================================
ExerciseSelector::ExerciseSelector()
{
    for (int i = 0; i < NUM_EXERCISES; i++) {
        juce::TextButton& exerciseButton = exerciseButtons[i];
        addAndMakeVisible(exerciseButton);
        exerciseButton.setButtonText("Exercise " + juce::String(i + 1));
        exerciseButton.onClick = [this, i] { if (onSelectExercise) onSelectExercise(i + 1); };
    }

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
    const int buttonHeight = 50;
    const int spacing = 20;
    int y = 0;

    for (int i = 0; i < NUM_EXERCISES; i++) {
        juce::TextButton& exerciseButton = exerciseButtons[i];
        exerciseButton.setBounds(0, y, getWidth(), buttonHeight);
        y += buttonHeight + spacing;
    }
}
