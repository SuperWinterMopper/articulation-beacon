#pragma once

#include <JuceHeader.h>
#include "Constants.h"

//==============================================================================
class ExerciseSelector  : public juce::Component
{
public:
    ExerciseSelector();
    ~ExerciseSelector() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    //this function is implemented on the MainComponent to switch to corresponding exercise
    std::function<void(ViewOptions newView)> onSelectExercise;

private:
    std::array<juce::TextButton, NUM_EXERCISES> exerciseButtons;
        
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExerciseSelector)
};
