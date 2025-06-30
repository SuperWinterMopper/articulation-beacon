/*
  ==============================================================================

    Exercise2.h
    Created: 30 Jun 2025 12:16:22pm
    Author:  afkhe

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Exercise2  : public juce::Component
{
public:
    Exercise2();
    ~Exercise2() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Exercise2)
};
