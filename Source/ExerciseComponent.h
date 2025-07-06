/*
  ==============================================================================

    ExerciseComponent.h
    Created: 6 Jul 2025 12:51:07pm
    Author:  afkhe

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class ExerciseComponent  : public juce::Component
{
public:
    ExerciseComponent();
    ~ExerciseComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExerciseComponent)
};
