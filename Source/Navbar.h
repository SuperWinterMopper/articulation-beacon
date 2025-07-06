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

    juce::Colour buttonNormalColor  = juce::Colour(0xF030cdca);
    juce::Colour buttonOverColor    = juce::Colour(0xB930cdca);
    juce::Colour buttonDownColor    = juce::Colour(0x9C30cdca);
                                    
    juce::ShapeButton homeButton{"Home", buttonNormalColor, buttonOverColor, buttonDownColor };
    juce::ShapeButton prevButton{"Previous", buttonNormalColor, buttonOverColor, buttonDownColor };
    juce::ShapeButton playButton{"Play", buttonNormalColor, buttonOverColor, buttonDownColor };
    juce::ShapeButton skipButton{"Skip", buttonNormalColor, buttonOverColor, buttonDownColor };
    juce::ShapeButton settingsButton{"Settings", buttonNormalColor, buttonOverColor, buttonDownColor };

    void Navbar::configureHomeButton();
    void Navbar::configurePrevButton();
    void Navbar::configurePlayButton();
    void Navbar::configureSkipButton();
    void Navbar::configureSettingsButton();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Navbar)
};
