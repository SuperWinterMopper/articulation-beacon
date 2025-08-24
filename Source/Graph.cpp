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
            processInputSample(sample);
        }
    }
}

void Graph::processInputSample(float sample) {
    fifo[fifoIndex] = sample;
    
    fifoIndex += 1;
    if (fifoIndex == fftSize)
        fifoIndex = 0;

    totalSamplesProcessed += 1;

    count += 1;
    if (count == hopLength) {
        count = 0;
        //make sure we've saturated our fifo (only guards against the very start)
        if(totalSamplesProcessed > fftSize) performAnalysis();
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

        bool ifScanFlux = processFluxSampleAndIfScan(flux);
        if (ifScanFlux) performFluxScan();
    }
    else {
        getMagnitudeSpectrogram(fftPtr, prevMags.data()); // fill prevMags with magnitude values
        prevMagsComputed = true;
    }
}

bool Graph::processFluxSampleAndIfScan(float rawFlux) {
    // Smooth this single frame
    const float smoothed = fluxSmoother.process(rawFlux);

    // Normalize with leaky-max (once per frame)
    const float normed = norm.update(smoothed);

    int64_t centerSample = (totalSamplesProcessed - 1) - (fftSize - 1) / 2;
    int64_t adjustedSample = centerSample - (int64_t)gaussDelayFrames * hopLength;

    if (adjustedSample < 0) adjustedSample = 0; // adjust to clamp during warm-up

    fluxFifo[fluxFifoIndex] = normed;
    fluxTimeFifo[fluxFifoIndex] = totalSamplesProcessed;
    fluxFifoIndex += 1;
    if (fluxFifoIndex == fluxSize)
        fluxFifoIndex = 0;
    
    fluxCount += 1;
    if (fluxCount == fluxHopLength) {
        fluxCount = 0;
        //make sure we've saturated our fluxFifo (only guards against the very start)
        if (totalSamplesProcessed > fftSize * fluxSize) return true;
    }
    return false;
}

void Graph::performFluxScan() {
    //Make fluxData [0... fluxSize - 1] of flux values, similar for fluxTimeData 
    copyFluxFifoToData();

    //Initially we are searching for next ONSET, not sustain yet
    bool searchSustain = false;
    int onsetInit = 0;

    for (int i = 1; i < fluxSize - 1; i++) {
       
        // Track when we drop below threshold (potential onset start)
        if (fluxData[i] < metaData.onsetThresh) onsetInit = i;

        if (!searchSustain) {
            // Ensure enough time since last onset
            if (snapShots.empty() || (fluxTimeData[i] - snapShots.back().onsetSample) > metaData.minSamplesBetweenNotes) {
                // Local peak detection
                if (fluxData[i] > fluxData[i - 1] && fluxData[i] >= fluxData[i + 1] && fluxData[i] > metaData.onsetThresh) {
                    ArticulationWindow window;
                    window.onsetSampleIndex = onsetInit;
                    window.onsetSample = fluxTimeData[onsetInit];

                    snapShots.push_back(window);
                    searchSustain = true;
                }
            }
        }
        else {
            // Look ahead window 
            int end = std::min(i + (int)minSamplesBetweenNotes, fluxSize);
            double avg = 0.0;
            int count = 0;
            for (int j = i; j < end; j++) {
                avg += fluxData[j];
                count++;
            }
            avg /= std::max(1, count);

            if (avg < metaData.sustainThresholdValue) {
                // Register sustain
                if (!snapShots.empty()) {
                    snapShots.back().sustainSampleIndex = i;
                    snapShots.back().sustainSample = fluxTimeData[i];
                }
                searchSustain = false;
            }
        }
    }
}

void Graph::copyFluxFifoToData() {
    //Put fifoFlux data into fluxData
    const float* fifoPtr = fluxFifo.data();
    float* dataPtr = fluxData.data();
    std::memcpy(dataPtr, fifoPtr + fluxFifoIndex, (fluxSize - fluxFifoIndex) * sizeof(float));
    if (fluxFifoIndex > 0)
        std::memcpy(dataPtr + fluxSize - fluxFifoIndex, fifoPtr, fluxFifoIndex * sizeof(float));

    //Put data into fluxTimeData
    const int64_t* tFifoPtr = fluxTimeFifo.data();
    int64_t* tDataPtr = fluxTimeData.data();
    std::memcpy(tDataPtr, tFifoPtr + fluxFifoIndex, (fluxSize - fluxFifoIndex) * sizeof(int64_t));
    if (fluxFifoIndex > 0)
        std::memcpy(tDataPtr + fluxSize - fluxFifoIndex, tFifoPtr, fluxFifoIndex * sizeof(int64_t));
}

void Graph::getMagnitudeSpectrogram(float* a_fftData, float* res) {
    //recast to complex numbers
    for (int i = 0; i < numBins; i++) 
        res[i] = a_fftData[i];
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
    
    fluxSmoother.reset();
    retrieveMetaData();
}

void Graph::retrieveMetaData() {
    jassert(false);
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
    fluxSmoother.reset();
}