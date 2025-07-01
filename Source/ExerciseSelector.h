/*
  ==============================================================================

    ExerciseSelector.h
    Created: 30 Jun 2025 8:15:56pm
    Author:  afkhe

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class ExerciseSelector  : public juce::Component
{
public:
    ExerciseSelector();
    ~ExerciseSelector() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::TextButton exercise1Button;
    juce::TextButton exercise2Button;
    juce::TextButton exercise3Button;
    juce::TextButton exercise4Button;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExerciseSelector)
};
