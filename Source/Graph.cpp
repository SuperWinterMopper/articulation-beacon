#include <JuceHeader.h>
#include "Graph.h"

Graph::Graph(juce::ValueTree scoreState, int exerciseDataIndex) : 
    fft(fftOrder), 
    scoreState(scoreState), 
    exerciseDataIndex(exerciseDataIndex), 
    metaData(ExerciseData[exerciseDataIndex]),
    hannWindow(fftSize + 1, juce::dsp::WindowingFunction<float>::WindowingMethod::hann, false)
{
    scoreState.addListener(this);

    startTimerHz(60); //this timer will be called every ~16ms and will trigger graph computation and rendering
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

void Graph::timerCallback() {
    if (nextFFTBlockReady) //false for now, but in future have a bool which indicates an articulation has been found, all computation are done, ready to show the graph
    {
        performAnalysis();
        nextFFTBlockReady = false;
        repaint(); //this is to show the potentially rendered graph
    }
}

void Graph::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (!scoreState.hasProperty(isAnalyzing))
        DBG("ISSUE: in Graph.cpp isAnalyzing not yet set even though component initialized ");
    //don't do anything if we aren't analyzing yet
    if (!scoreState.getProperty(isAnalyzing))
        return;

    if (bufferToFill.buffer->getNumChannels() > 0)
    {
        auto* channelData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
        for (auto i = 0; i < bufferToFill.numSamples; i++) {
            float sample = channelData[i];
            processSample(sample);
        }
        totalSamplesProcessed += bufferToFill.numSamples;
    }
}

void Graph::processSample(float sample) {
    fifo[fifoIndex] = sample;
    
    fifoIndex += 1;
    if (fifoIndex == fftSize)
        fifoIndex = 0;

    count += 1;
    if (count == hopLength) {
        count = 0;
        performAnalysis();
    }
}

void Graph::performAnalysis() {

    //Copy data from fifo to fftData
    const float* inputPtr = fifo.data();
    float* fftPtr = fftData.data();
    std::memcpy(fftPtr, inputPtr + fifoIndex, (fftSize - fifoIndex) * sizeof(float));
    if (fifoIndex > 0) 
        std::memcpy(fftPtr + fftSize - fifoIndex, inputPtr, fifoIndex * sizeof(float));
    
    // Apply Hann windowing to avoid spectral leakage.
    hannWindow.multiplyWithWindowingTable(fftPtr, fftSize);
    fft.performFrequencyOnlyForwardTransform(fftData.data(), true); //compute FFT of only magnitude values

    //Retrieve magnitude spectorgram
    if (prevMagsComputed) {
        getMagnitudeSpectrogram(fftPtr, newMags.data()); // fill prevMags with magnitude values
        int flux = computeFluxValue(newMags.data(), prevMags.data());
        std::swap(prevMags, newMags);
    }
    else {
        getMagnitudeSpectrogram(fftPtr, prevMags.data()); // fill prevMags with magnitude values
        prevMagsComputed = true;
    }

}

void Graph::getMagnitudeSpectrogram(float* a_fftData, float* res) {
    //recast to complex numbers
    auto* cdata = reinterpret_cast<std::complex<float>*>(a_fftData);
    for (int i = 0; i < numBins; i++) {
        float mag = std::abs(cdata[i]); //absolute value 
        res[i] = mag;
    }
}

float Graph::computeFluxValue(float* cur, float* prev) {
    float ret = 0;
    for (int i = 0; i < numBins; i++)
        ret += std::max(cur[i] - prev[i], 0.0f); //half wave rectify
    return ret;
}

void Graph::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    if (tree == scoreState) {
        if (property == tempo) {
            metaData = ExerciseData[exerciseDataIndex + (int)scoreState.getProperty(tempo)]; //update metaData if we switch to faster or slower tempo
        }
        if (property == isAnalyzing) {
            totalSamplesProcessed = 0; //whenever change state in analysis status reset totalSamplesProcessed to 0
        }
    }
}

void Graph::resized()
{

}

void Graph::prepareToPlay(int samplesPerBlockExpected, double sr)
{
    //probably read in precomputed target recording data here
    sampleRate = sr;
    secondsPerSample = 1.0 / sampleRate;

    //initialize frequency bins:
    double binSpacing = (double) sampleRate / fftSize;
    for (int i = 0; i < fftSize; i++) 
        freqBins[i] = i * binSpacing;
}

void Graph::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

void Graph::reset() {
    fifoIndex = 0;
    count = 0;
    std::fill(fifo.begin(), fifo.end(), 0.0f);
}