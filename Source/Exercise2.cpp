#include <JuceHeader.h>
#include "Exercise2.h"

//==============================================================================
Exercise2::Exercise2()
{
    addAndMakeVisible(navBar);

}

Exercise2::~Exercise2()
{

}

void Exercise2::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("Exercise2", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void Exercise2::resized()
{
    const int navBarHeight = 100;

    navBar.setBounds(0, getHeight() - navBarHeight, getWidth(), navBarHeight);


}
