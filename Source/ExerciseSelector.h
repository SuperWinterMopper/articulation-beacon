#pragma once

#include <JuceHeader.h>

//==============================================================================
class ExerciseSelector  : public juce::Component
{
public:
    ExerciseSelector();
    ~ExerciseSelector() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    //this function is implemented on the MainComponent to switch to corresponding exercise
    std::function<void(int)> onSelectExercise;

private:
    static const std::size_t NUM_EXERCISES = 10;

    std::array<juce::TextButton, NUM_EXERCISES> exerciseButtons;
        
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExerciseSelector)
};
