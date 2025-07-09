#pragma once

#include <JuceHeader.h>
#include "Navbar.h"

//==============================================================================
class ExerciseComponent  : public juce::Component
{
public:
    ExerciseComponent(std::string name);
    ~ExerciseComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    //this function is called by the home button to get back to home screen
    std::function<void()> homeButtonClick;

private:
    Navbar navBar;

    std::string this_name;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExerciseComponent)
};
