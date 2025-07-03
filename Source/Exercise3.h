/*
  ==============================================================================

    Exercise3.h
    Created: 2 Jul 2025 7:58:39pm
    Author:  afkhe

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Exercise3  : public juce::Component
{
public:
    Exercise3();
    ~Exercise3() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Exercise3)
};
