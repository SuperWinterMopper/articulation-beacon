#pragma once

#include <JuceHeader.h>
#include "Constants.h"
#include "ExerciseSelector.h"
#include "Exercise1.h"
#include "Exercise2.h"
#include "Exercise3.h"
#include "Exercise4.h"

class ABLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ABLookAndFeel()
    {
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, bool isMouseOverButton, bool isButtonDown) override
    {
        auto buttonArea = button.getLocalBounds().toFloat();
        auto edge = 4;
        float cornerSize = 12.0f;

        buttonArea.removeFromLeft(edge);
        buttonArea.removeFromTop(edge);

        auto offset = isButtonDown ? -edge / 2 : -edge;
        buttonArea.translate(offset, offset);

        g.setColour(backgroundColour);
        g.fillRoundedRectangle(buttonArea, cornerSize);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool isMouseOverButton, bool isButtonDown) override
    {
        auto font = getTextButtonFont(button, button.getHeight());
        g.setFont(font);
        g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId
            : juce::TextButton::textColourOffId)
            .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f));

        auto yIndent = juce::jmin(4, button.proportionOfHeight(0.3f));
        auto cornerSize = juce::jmin(button.getHeight(), button.getWidth()) / 2;

        auto fontHeight = juce::roundToInt(font.getHeight() * 0.6f);
        auto leftIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnLeft() ? 4 : 2));
        auto rightIndent = juce::jmin(fontHeight, 2 + cornerSize / (button.isConnectedOnRight() ? 4 : 2));
        auto textWidth = button.getWidth() - leftIndent - rightIndent;

        auto edge = 4;
        auto offset = isButtonDown ? edge / 2 : 0;

        if (textWidth > 0)
            g.drawFittedText(button.getButtonText(),
                leftIndent + offset, yIndent + offset, textWidth, button.getHeight() - yIndent * 2 - edge,
                juce::Justification::centred, 2);
    }

    juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) {
        return juce::Font((float)buttonHeight * .75f);
    }
};

class MainComponent : public juce::Component
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    //expose all exercises
    std::array<std::unique_ptr<juce::Component>, NUM_EXERCISES> exercises;

    void viewSwitch(juce::Component* newView);

    juce::Component* curView;

private:
    //Actual pages that will be opened for each exercise
    Exercise1 exercise1Page;
    Exercise2 exercise2Page;
    Exercise3 exercise3Page;
    Exercise4 exercise4Page;

    //"ARTICULATION BEACON" logo
    juce::Label appTitle;

    //design of app overall is defined here
    ABLookAndFeel ABLook;

    //Button selector
    ExerciseSelector exerciseSelector;

    std::unique_ptr<juce::FileLogger> fileLogger;

    //Sets up logger at the application Data Directory (Appdata/Roaming/ for Windows, Library/Application Support/ for Mac
    void setUpLogger();

    void viewOpener(int exerciseNum);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};