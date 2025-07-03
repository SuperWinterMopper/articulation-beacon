/*
  ==============================================================================

    Exercise3.cpp
    Created: 2 Jul 2025 7:58:39pm
    Author:  afkhe

  ==============================================================================
*/

#include <JuceHeader.h>
#include "Exercise3.h"

//==============================================================================
Exercise3::Exercise3()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

Exercise3::~Exercise3()
{
}

void Exercise3::paint (juce::Graphics& g)
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
    g.drawText ("Exercise3", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void Exercise3::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
