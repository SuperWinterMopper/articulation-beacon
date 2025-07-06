#pragma once

#include <JuceHeader.h>
#include "Navbar.h"

//==============================================================================
class Exercise2  : public juce::Component
{
public:
    Exercise2();
    ~Exercise2() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    //this function is called by the home button to get back to home screen
    std::function<void()> homeButtonClick;
private:
    Navbar navBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Exercise2)
};