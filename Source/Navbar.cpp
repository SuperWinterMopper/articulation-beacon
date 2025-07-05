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
    ////where SVG should be at
    //juce::File iconFileLocation = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("Resources/homeButtonIcon.svg");

    //auto drawable = loadSVG(iconFileLocation);

    //DBG(iconFileLocation.getFullPathName());
    //juce::Logger::writeToLog("iconFileLocation is " + iconFileLocation.getFullPathName());
    //jassert(drawable);

    //auto normal = drawable->createCopy();
    //auto over = drawable->createCopy();
    //auto down = drawable->createCopy();

    //over->setAlpha(0.85f);
    //down->setAlpha(0.70f);

    //homeButton.setImages(std::move (),
    //    over.release(),
    //    down.release(),
    //    nullptr, nullptr, nullptr);
}

void Navbar::configurePrevButton()
{
    ////where SVG should be at
    //juce::File iconFileLocation = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("Resources/prevButtonIcon.svg");

    //auto drawable = loadSVG(iconFileLocation);

    //DBG(iconFileLocation.getFullPathName());
    //juce::Logger::writeToLog("iconFileLocation is " + iconFileLocation.getFullPathName());
    //jassert(drawable);

    //auto normal = drawable->createCopy();
    //auto over = drawable->createCopy();
    //auto down = drawable->createCopy();

    //over->setAlpha(0.85f);
    //down->setAlpha(0.70f);

    //prevButton.setImages(normal.release(),
    //    over.release(),
    //    down.release(),
    //    nullptr, nullptr, nullptr);

    //prevButton.setImages ( std::move (normal),
    //                       std::move (over),
    //                       std::move (down) );

}

void Navbar::configurePlayButton()
{

}

void Navbar::configureSkipButton()
{

}


void Navbar::configureSettingsButton()
{
    static const unsigned char pathData[] = { 110,109,206,170,118,65,179,170,18,66,108,0,0,108,65,206,170,250,65,98,239,56,102,65,86,142,249,65,17,199,96,65,239,56,248,65,206,170,91,65,206,170,246,65,98,34,142,86,65,120,28,245,65,34,142,81,65,222,113,243,65,206,170,76,65,206,170,241,65,108,164,170,
    250,64,179,42,1,66,108,0,0,80,64,0,0,195,65,108,92,85,241,64,0,0,169,65,98,201,113,240,64,222,113,167,65,0,0,240,64,222,241,165,65,0,0,240,64,0,128,164,65,108,0,0,240,64,0,128,155,65,98,0,0,240,64,86,14,154,65,201,113,240,64,86,142,152,65,92,85,241,64,
    0,0,151,65,108,0,0,80,64,0,0,122,65,108,164,170,250,64,248,170,246,64,108,206,170,76,65,216,170,28,65,98,34,142,81,65,162,28,25,65,206,170,86,65,69,199,21,65,0,0,92,65,216,170,18,65,98,50,85,97,65,97,142,15,65,206,170,102,65,189,227,12,65,0,0,108,65,
    216,170,10,65,108,206,170,118,65,239,85,85,64,108,153,170,196,65,239,85,85,64,108,0,0,202,65,216,170,10,65,98,136,227,204,65,189,227,12,65,120,156,207,65,97,142,15,65,153,42,210,65,216,170,18,65,98,239,184,212,65,69,199,21,65,239,56,215,65,162,28,25,
    65,153,170,217,65,216,170,28,65,108,179,170,0,66,248,170,246,64,108,0,0,19,66,0,0,122,65,108,77,213,1,66,0,0,151,65,98,196,241,1,66,86,142,152,65,0,0,2,66,86,14,154,65,0,0,2,66,0,128,155,65,108,0,0,2,66,0,128,164,65,98,0,0,2,66,222,241,165,65,136,227,
    1,66,222,113,167,65,179,170,1,66,0,0,169,65,108,77,213,18,66,0,0,195,65,108,0,128,0,66,179,42,1,66,108,153,170,217,65,206,170,241,65,98,239,56,215,65,222,113,243,65,153,170,212,65,120,28,245,65,0,0,210,65,206,170,246,65,98,103,85,207,65,239,56,248,65,
    153,170,204,65,86,142,249,65,0,0,202,65,206,170,250,65,108,153,170,196,65,179,170,18,66,108,206,170,118,65,179,170,18,66,99,109,153,170,160,65,206,170,206,65,98,34,142,173,65,206,170,206,65,34,142,184,65,120,28,202,65,153,170,193,65,0,0,193,65,98,17,
    199,202,65,189,227,183,65,103,85,207,65,189,227,172,65,103,85,207,65,0,0,160,65,98,103,85,207,65,120,28,147,65,17,199,202,65,120,28,136,65,153,170,193,65,0,0,126,65,98,34,142,184,65,122,199,107,65,34,142,173,65,206,170,98,65,153,170,160,65,206,170,98,
    65,98,34,142,147,65,206,170,98,65,0,128,136,65,122,199,107,65,0,0,127,65,0,0,126,65,98,0,0,109,65,120,28,136,65,0,0,100,65,120,28,147,65,0,0,100,65,0,0,160,65,98,0,0,100,65,189,227,172,65,0,0,109,65,189,227,183,65,0,0,127,65,0,0,193,65,98,0,128,136,65,
    120,28,202,65,34,142,147,65,206,170,206,65,153,170,160,65,206,170,206,65,99,101,0,0 };

    juce::Path path;
    path.loadPathFromData(pathData, sizeof(pathData));

    settingsButton.setShape(path, true, true, true); 
    settingsButton.setColours(juce::Colour(0xF030cdca), juce::Colour(0xF630cdca), juce::Colour(0xFF30cdca));
}