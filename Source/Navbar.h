#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class Navbar  : public juce::Component
{
public:
    Navbar();
    ~Navbar() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //note each button is a square so height = width
    const int buttonWidth = 40;

    juce::DrawableButton homeButton{ "Home", juce::DrawableButton::ImageOnButtonBackground };
    juce::DrawableButton prevButton{ "Previous", juce::DrawableButton::ImageOnButtonBackground };
    juce::ShapeButton playButton{ "Play", juce::Colours::blue, juce::Colours::lightblue, juce::Colours::darkblue };
    juce::ShapeButton skipButton{ "Skip", juce::Colours::blue, juce::Colours::lightblue, juce::Colours::darkblue };
    juce::ShapeButton settingsButton{ "Settings", juce::Colours::blue, juce::Colours::lightblue, juce::Colours::darkblue };

    void Navbar::configureHomeButton();
    void Navbar::configurePrevButton();
    void Navbar::configurePlayButton();
    void Navbar::configureSkipButton();
    void Navbar::configureSettingsButton();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Navbar)
};
