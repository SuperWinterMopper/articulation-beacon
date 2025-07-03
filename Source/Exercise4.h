/*
  ==============================================================================

    Exercise4.h
    Created: 2 Jul 2025 7:59:04pm
    Author:  afkhe

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Exercise4  : public juce::Component
{
public:
    Exercise4();
    ~Exercise4() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Exercise4)
};
