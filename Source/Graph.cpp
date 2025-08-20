#include <JuceHeader.h>
#include "Graph.h"

Graph::Graph(juce::ValueTree scoreState) : fft(fftOrder), scoreState(scoreState)
{
    scoreState.addListener(this);
}

Graph::~Graph()
{

}

void Graph::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("Graph", getLocalBounds(), juce::Justification::centred, true);   // draw some placeholder text
}

void Graph::resized()
{

}

void Graph::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    //probably read in precomputed target recording data here
}

void Graph::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    DBG("called graph insdie");
}


void Graph::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (bufferToFill.buffer->getNumChannels() > 0)
    {
        auto* channelData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
        for (auto i = 0; i < bufferToFill.numSamples; i++) {
            float sample = channelData[i];

            if (fifoIndex == fftSize) {
                if (!nextFFTBlockReady) {
                    std::fill(fftData.begin(), fftData.end(), 0.0f);
                    std::copy(fifo.begin(), fifo.end(), fftData.begin());
                    nextFFTBlockReady = true;
                }
                fifoIndex = 0;
            }
            else
                fifo[fifoIndex++] = sample;
        }

    }
}

void Graph::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}
