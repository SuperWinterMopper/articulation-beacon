/*
  ==============================================================================

    Exercise4.cpp
    Created: 2 Jul 2025 7:59:04pm
    Author:  afkhe

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Exercise4.h"

//==============================================================================
Exercise4::Exercise4()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

Exercise4::~Exercise4()
{
}

void Exercise4::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("Exercise4", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void Exercise4::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
