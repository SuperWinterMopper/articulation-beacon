#include <JuceHeader.h>
#include "Navbar.h"

//==============================================================================
Navbar::Navbar()
{
    configureHomeButton();
    configurePrevButton();
    configurePlayButton();
    configureSkipButton();
    configureSettingsButton();

    addAndMakeVisible(homeButton);
    addAndMakeVisible(prevButton);
    addAndMakeVisible(playButton);
    addAndMakeVisible(skipButton);
    addAndMakeVisible(settingsButton);

    //this is a default size but will be overidden by parent Exercise components
    setSize(100, 800);
}

Navbar::~Navbar()
{

}

void Navbar::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("Navbar", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void Navbar::resized()
{
    int XForCenterButton = (getWidth() / 2) - (buttonWidth / 2), YForCenterButton = (getHeight() / 2) - (buttonWidth / 2);
    int buttonSpacing = 30;

    //this seems archaic but I'm not learning flexbox for juce
    homeButton.setBounds(XForCenterButton - 2 * buttonSpacing - 2 * buttonWidth, YForCenterButton, this->buttonWidth, this->buttonWidth);
    prevButton.setBounds(XForCenterButton - 1 * buttonSpacing - 1 * buttonWidth, YForCenterButton, this->buttonWidth, this->buttonWidth);
    playButton.setBounds(XForCenterButton - 0 * buttonSpacing - 0 * buttonWidth, YForCenterButton, this->buttonWidth, this->buttonWidth);
    skipButton.setBounds(XForCenterButton + 1 * buttonSpacing + 1 * buttonWidth, YForCenterButton, this->buttonWidth, this->buttonWidth);
    settingsButton.setBounds(XForCenterButton + 2 * buttonSpacing + 2 * buttonWidth, YForCenterButton, this->buttonWidth, this->buttonWidth);
}

//helper function to load SVG
static std::unique_ptr<juce::Drawable> loadSVG(const juce::File& file)
{
    std::unique_ptr<juce::XmlElement> svg(juce::XmlDocument::parse(file));
    return svg ? juce::Drawable::createFromSVG(*svg) : nullptr;
}

void Navbar::configureHomeButton()
{
    //where SVG should be at
    juce::File iconFileLocation = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("Resources/homeButtonIcon.svg");

    auto drawable = loadSVG(iconFileLocation);


    DBG(iconFileLocation.getFullPathName());
    juce::Logger::writeToLog("iconFileLocation is " + iconFileLocation.getFullPathName());
    jassert(drawable);

    auto normal = std::unique_ptr<juce::Drawable>(drawable->createCopy());
    auto over = std::unique_ptr<juce::Drawable>(drawable->createCopy());
    auto down = std::unique_ptr<juce::Drawable>(drawable->createCopy());

    over->setAlpha(0.85f);
    down->setAlpha(0.70f);

    homeButton.setImages(normal.release(),
        over.release(),
        down.release(),
        nullptr, nullptr, nullptr);
}

void Navbar::configurePrevButton()
{
    //where SVG should be at
    juce::File iconFileLocation = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("Resources/prevButtonIcon.svg");

    auto drawable = loadSVG(iconFileLocation);

    DBG(iconFileLocation.getFullPathName());
    juce::Logger::writeToLog("iconFileLocation is " + iconFileLocation.getFullPathName());
    jassert(drawable);

    auto normal = std::unique_ptr<juce::Drawable>(drawable->createCopy());
    auto over = std::unique_ptr<juce::Drawable>(drawable->createCopy());
    auto down = std::unique_ptr<juce::Drawable>(drawable->createCopy());

    over->setAlpha(0.85f);
    down->setAlpha(0.70f);

    prevButton.setImages(normal.release(),
        over.release(),
        down.release(),
        nullptr, nullptr, nullptr);
}

void Navbar::configurePlayButton()
{

}

void Navbar::configureSkipButton()
{

}


void Navbar::configureSettingsButton()
{

}