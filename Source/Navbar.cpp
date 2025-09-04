#include <JuceHeader.h>
#include "Navbar.h"

//==============================================================================
Navbar::Navbar(juce::ValueTree scoreState, ViewOptions thisComponentView) : scoreState(scoreState), thisComponentView(thisComponentView)
{
    configureHomeButton();
    configurePrevButton();
    configurePlayButton();
    configureSkipButton();
    configureSettingsButton();
    configureHearPlayButton();
    configureTempoButton();

    addAndMakeVisible(homeButton);
    addAndMakeVisible(prevButton);
    addAndMakeVisible(playButton);
    addAndMakeVisible(skipButton);
    addAndMakeVisible(settingsButton);
    addAndMakeVisible(hearPlayButton);
    addAndMakeVisible(tempoButton);

    //this is a default size but will be overidden by parent Exercise components
    setSize(100, 800);
}

Navbar::~Navbar()
{

}

void Navbar::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xFF3D3D3D));
}

void Navbar::resized()
{
    if (getWidth() <= 0 || getHeight() <= 0)
        return;

    int XForCenterButton = (getWidth() / 2) - (buttonWidth / 2);
    int YForCenterButton = (getHeight() / 2) - (buttonWidth / 2);
    int buttonSpacing = 80;

    // Keep existing five buttons centered exactly as before
    homeButton.setBounds(XForCenterButton - 2 * buttonSpacing - 2 * buttonWidth, YForCenterButton, buttonWidth, buttonWidth);
    prevButton.setBounds(XForCenterButton - 1 * buttonSpacing - 1 * buttonWidth, YForCenterButton, buttonWidth, buttonWidth);
    playButton.setBounds(XForCenterButton - 0 * buttonSpacing - 0 * buttonWidth, YForCenterButton, buttonWidth, buttonWidth);
    skipButton.setBounds(XForCenterButton + 1 * buttonSpacing + 1 * buttonWidth, YForCenterButton, buttonWidth, buttonWidth);
    settingsButton.setBounds(XForCenterButton + 2 * buttonSpacing + 2 * buttonWidth, YForCenterButton, buttonWidth, buttonWidth);

    const auto textH = buttonWidth; // keep height consistent with icon buttons

    auto fontHP = getLookAndFeel().getTextButtonFont(hearPlayButton, textH);
    auto fontTmp = getLookAndFeel().getTextButtonFont(tempoButton, textH);

    const int hearPlayWidth = juce::jmax(buttonWidth,
        (int)std::ceil(fontHP.getStringWidthFloat("Hear then Play")) + 16); // + padding

    const int tempoWidth = juce::jmax(buttonWidth,
        (int)std::ceil(fontTmp.getStringWidthFloat("120bpm")) + 16);

    const int xAfterSettings = settingsButton.getRight() + buttonSpacing;

    hearPlayButton.setBounds(xAfterSettings, YForCenterButton, hearPlayWidth, textH);
    tempoButton.setBounds(hearPlayButton.getRight() + buttonSpacing, YForCenterButton, tempoWidth, textH);

}

void Navbar::configureHomeButton()
{
    juce::Path path;
    // Build a simple house: roof + body + door in a 100x100 coordinate space
    path.addTriangle(50.0f, 10.0f, 15.0f, 40.0f, 85.0f, 40.0f); // roof
    path.addRectangle(25.0f, 40.0f, 50.0f, 50.0f);               // body
    path.addRectangle(45.0f, 60.0f, 10.0f, 30.0f);               // door
    const float scale = (float)buttonWidth / 100.0f;
    path.applyTransform(juce::AffineTransform::scale(scale));

    homeButton.setSize(buttonWidth, buttonWidth);
    homeButton.setShape(path, false, true, false);

    //button calls `homeButtonClick()` when clicked. This is implemented by the MainComponent
    homeButton.onClick = [this] { if (homeButtonClick) homeButtonClick(); };
}

void Navbar::configurePrevButton()
{
    juce::Path path;
    // Left-pointing triangle (prev)
    path.addTriangle(70.0f, 20.0f, 30.0f, 50.0f, 70.0f, 80.0f);
    path.applyTransform(juce::AffineTransform::scale((float)buttonWidth / 100.0f));

    prevButton.setSize(buttonWidth, buttonWidth);
    prevButton.setShape(path, false, true, false);
}

void Navbar::configurePlayButton()
{
    juce::Path path;
    // Right-pointing triangle (play)
    path.addTriangle(30.0f, 20.0f, 30.0f, 80.0f, 75.0f, 50.0f);
    path.applyTransform(juce::AffineTransform::scale((float)buttonWidth / 100.0f));

    playButton.setSize(buttonWidth, buttonWidth);
    playButton.setShape(path, false, true, false);

    //Switch video playing 
    playButton.onClick = [this] {scoreState.setProperty(isMetronomePlaying, !scoreState[isMetronomePlaying], nullptr); };
}

void Navbar::configureSkipButton()
{
    juce::Path path;
    // Right-pointing triangle (skip)
    path.addTriangle(30.0f, 20.0f, 75.0f, 50.0f, 30.0f, 80.0f);
    path.applyTransform(juce::AffineTransform::scale((float)buttonWidth / 100.0f));

    skipButton.setSize(buttonWidth, buttonWidth);
    skipButton.setShape(path, false, true, false);
}


void Navbar::configureSettingsButton()
{
    juce::Path path;
    // Simple gear-like shape: outer circle + inner hole + 6 small protrusions
    path.addEllipse(20.0f, 20.0f, 60.0f, 60.0f);
    for (int i = 0; i < 6; ++i)
    {
        const float angle = juce::MathConstants<float>::twoPi * (i / 6.0f);
        const float cx = 50.0f + 35.0f * std::cos(angle);
        const float cy = 50.0f + 35.0f * std::sin(angle);
        juce::Path tooth;
        tooth.addRectangle(-5.0f, -10.0f, 10.0f, 20.0f);
        tooth.applyTransform(juce::AffineTransform::rotation(angle).translated(cx, cy));
        path.addPath(tooth);
    }
    // inner hole
    juce::Path hole; hole.addEllipse(40.0f, 40.0f, 20.0f, 20.0f);
    path.addPath(hole);
    path.setUsingNonZeroWinding(false); // make the hole visible
    path.applyTransform(juce::AffineTransform::scale((float)buttonWidth / 100.0f));

    settingsButton.setSize(buttonWidth, buttonWidth);
    settingsButton.setShape(path, false, true, false);
}

void Navbar::configureHearPlayButton()
{
    // Style roughly aligned with your colour scheme
    hearPlayButton.setColour(juce::TextButton::buttonColourId, buttonNormalColor);
    hearPlayButton.setColour(juce::TextButton::buttonOnColourId, buttonDownColor);
    hearPlayButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    hearPlayButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);

    hearPlayButton.onClick = [this]
        {
            auto current = scoreState.getProperty(userMode).toString();
            if (current == "Hear then Play") {
                hearPlayButton.setButtonText("Play");
                scoreState.setProperty(userMode, "Play", nullptr);
            }
            else if (current == "Play") {
                hearPlayButton.setButtonText("Hear then Play");
                scoreState.setProperty(userMode, "Hear then Play", nullptr);
            }
            else {
                DBG("userMode incorrect inside Navbar");
                jassert(false);
            }
        };
}

void Navbar::configureTempoButton()
{
    int exerciseNum = static_cast<int>(thisComponentView) - 1; //convert enum value to int and make 0-indexed

    if (exerciseNum < 0 || exerciseNum >= (int)exerciseTempo.size())
    {
        DBG("Navbar::configureTempoButton invalid exercise index: " << exerciseNum);
        tempoButton.setEnabled(false);
        tempoButton.setButtonText("-");
        return;
    }
    const auto& tempos = exerciseTempo[exerciseNum];
    if (tempos.empty())
    {
        DBG("Navbar::configureTempoButton no tempos for exercise index: " << exerciseNum);
        tempoButton.setEnabled(false);
        tempoButton.setButtonText("-");
        return;
    }

    int slowTempo = tempos.front(); // initially 0 index
    tempoButton.setButtonText(std::to_string(slowTempo) + "bpm");

    tempoButton.setColour(juce::TextButton::buttonColourId, buttonNormalColor);
    tempoButton.setColour(juce::TextButton::buttonOnColourId, buttonDownColor);
    tempoButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    tempoButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);

    tempoButton.onClick = [this]
        {
            // convert enum value to int and make 0-indexed
            int exerciseNum = static_cast<int>(thisComponentView) - 1;
            if (exerciseNum < 0 || exerciseNum >= (int)exerciseTempo.size())
                return;

            // this exercise doesn't have a fast version, don't do anything
            if (exerciseTempo[exerciseNum].size() < 2)
                return;
                
            // switch to opposite tempo (slow to fast, fast to slow)
            scoreState.setProperty(tempo, int(scoreState.getProperty(tempo)) == 0 ? 1 : 0, nullptr);

            int idx = juce::jlimit(0, (int)exerciseTempo[exerciseNum].size() - 1, (int)scoreState.getProperty(tempo));
            int newTempo = exerciseTempo[exerciseNum][idx];
            tempoButton.setButtonText(std::to_string(newTempo) + "bpm");
        };
}