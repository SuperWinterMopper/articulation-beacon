#pragma once

#include <JuceHeader.h>
#include "ExerciseSelector.h"
#include "ExerciseComponent.h"

#if JUCE_WINDOWS
    #include <windows.h>
    #include <comdef.h>  
#endif


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

class MainComponent : public juce::AudioAppComponent
{
public:
    MainComponent();
    ~MainComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

private:
    //"ARTICULATION BEACON" logo
    juce::Label appTitle;

    //INVARIANT: curView is the current view of the app
    juce::ValueTree curView {viewState};

    //Array containing all the exercises
    std::array<ExerciseComponent, NUM_EXERCISES> exercisesArray { {
        {0, ViewOptions::EX1, curView},
        {1, ViewOptions::EX2, curView},
        {2, ViewOptions::EX3, curView},
        {3, ViewOptions::EX4, curView},
        {4, ViewOptions::EX5, curView},
        {5, ViewOptions::EX6, curView},
        {6, ViewOptions::EX7, curView},
        {7, ViewOptions::EX8, curView},
        {8, ViewOptions::EX9, curView},
        {9, ViewOptions::EX10, curView}
    } };

    int currentExerciseIndex = -1; // -1 means HOME

    bool exerciseComponentsReady = false;

    std::unique_ptr<juce::FileLogger> fileLogger; 

    #if JUCE_WINDOWS
        bool comInitialized = false;
    #endif

    //Button selector
    ExerciseSelector exerciseSelector;

    //design of app overall is defined here
    ABLookAndFeel ABLook;

    //Sets up logger at the application Data Directory (Appdata/Roaming/ for Windows, Library/Application Support/ for Mac
    void setUpLogger();

    //Handles view switching logic
    void viewSwitch(ViewOptions newView);

    //attaches all the exercise components initially (required for viewSwitch to work)
    void setUpExerciseComponents();

    //sets up COM for Windows devices 
    void initializeCOM();

    //sets up input and output channels
    void configInputOutput();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};