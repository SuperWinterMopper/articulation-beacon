#include <JuceHeader.h>
#include "Graph.h"

Graph::Graph(juce::ValueTree scoreState, int exerciseDataIndex) : 
    fft(fftOrder), 
    scoreState(scoreState), 
    exerciseDataIndex(exerciseDataIndex), 
    hannWindow(fftSize + 1, juce::dsp::WindowingFunction<float>::WindowingMethod::hann, false),
    spectrogramImage(juce::Image::RGB, 512, 512, true)
{
    scoreState.addListener(this);

    startTimerHz(60); //this timer will be called every ~16ms and will trigger graph computation and rendering

    //reserve memory. snapShots.resize because compiler is shitty and can't parse normal size
    snapShots.resize(abstractFifoCapacity);
    foundOnsetSamples.reserve(abstractFifoCapacity);

}

Graph::~Graph()
{
    stopTimer();
    scoreState.removeListener(this);
}


void Graph::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background
    
    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
    
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (14.0f));
    g.drawText ("Graph", getLocalBounds(), juce::Justification::centred, true);   // draw some placeholder text

    g.setOpacity(1.0f);
    g.drawImage(spectrogramImage, getLocalBounds().toFloat());
}

void Graph::timerCallback() {
    jassert(snapShots.size() == abstractFifoCapacity);

    if (nextFFTBlockReady) {
        drawNextLineOfSpectrogram();
        nextFFTBlockReady = false;
        repaint();
    }

    int s1, n1, s2, n2;
    
    //read as many as possible from the queue. shouldn't be more than a few, probably much less
    abstractArtEventFifo.prepareToRead(abstractArtEventFifo.getNumReady(), s1, n1, s2, n2);
    for(int i = 0; i < n1; i++) renderSnapShotGraph( snapShots[s1 + i]);
    for(int i = 0; i < n2; i++) renderSnapShotGraph( snapShots[s2 + i]);
    abstractArtEventFifo.finishedRead(n1 + n2);
}

void Graph::drawNextLineOfSpectrogram() {
    auto rightHandEdge = spectrogramImage.getWidth() - 1;
    auto imageHeight = spectrogramImage.getHeight();

    // first, shuffle our image leftwards by 1 pixel..
    spectrogramImage.moveImageSection(0, 0, 1, 0, rightHandEdge, imageHeight);         // [1]

    // then render our FFT data..
    fft.performFrequencyOnlyForwardTransform(fftData.data());                   // [2]

    // find the range of values produced, so we can scale our rendering to
    // show up the detail clearly
    auto maxLevel = juce::FloatVectorOperations::findMinAndMax(fftData.data(), fftSize / 2); // [3]

    juce::Image::BitmapData bitmap{ spectrogramImage, rightHandEdge, 0, 1, imageHeight, juce::Image::BitmapData::writeOnly }; // [4]

    for (auto y = 1; y < imageHeight; ++y)                                              // [5]
    {
        auto skewedProportionY = 1.0f - std::exp(std::log((float)y / (float)imageHeight) * 0.2f);
        auto fftDataIndex = (size_t)juce::jlimit(0, fftSize / 2, (int)(skewedProportionY * fftSize / 2));
        auto level = juce::jmap(fftData[fftDataIndex], 0.0f, juce::jmax(maxLevel.getEnd(), 1e-5f), 0.0f, 1.0f);

        bitmap.setPixelColour(0, y, juce::Colour::fromHSV(level, 1.0f, level, 1.0f)); // [6]
    }
}

void Graph::renderSnapShotGraph(const ArticulationWindow& window) {
    //to do: render graph of user articulation + match to closest in target recording
    //for now let's just make sure note onset detection works
    DBG("***************** NOTE ONSET DETECTED. GRAPH WILL DISPLAY FOR THAT ARTICULATION. ***********************");
    repaint();
}


void Graph::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // DBG("getNextAudioBlock of Graph.cpp called");
    if (!scoreState.hasProperty(isAnalyzing))
        DBG("ISSUE: in Graph.cpp isAnalyzing not yet set even though component initialized ");
    //don't do anything if we aren't analyzing yet
    if (!scoreState.getProperty(isAnalyzing))
        return;

    auto* channelData = bufferToFill.buffer->getReadPointer(0, bufferToFill.startSample);
    for (auto i = 0; i < bufferToFill.numSamples; i++) {
        float sample = channelData[i];
        processInputSample(sample);
    }
}

void Graph::processInputSample(float sample) {
    if (!scoreState.hasProperty(isAnalyzing))
        DBG("ISSUE: in Graph.cpp isAnalyzing not yet set even though component initialized ");
    //don't do anything if we aren't analyzing yet
    if (!scoreState.getProperty(isAnalyzing))
        return;

    fifo[fifoIndex] = sample;
    
    fifoIndex += 1;
    if (fifoIndex == fftSize) {
        fifoIndex = 0;
        // if (!nextFFTBlockReady) {
        //     std::fill(fftData.begin(), fftData.end(), 0.0f);
        //     std::copy(fifo.begin(), fifo.end(), fftData.begin());
        //     nextFFTBlockReady = true;
        // }
    }
    
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
    std::fill(fftPtr + fftSize, fftPtr + 2 * fftSize, 0.0f); //zero out the upper half
    
    float amp = 0.0f;
    for (int k = 0; k < fftSize; ++k) amp += std::abs(fftPtr[k]);
    amp /= float(fftSize);
    // Apply Hann windowing to avoid spectral leakage.
    hannWindow.multiplyWithWindowingTable(fftPtr, fftSize);
    fft.performFrequencyOnlyForwardTransform(fftData.data(), true); //compute FFT of only magnitude values

    //Retrieve magnitude spectorgram

    if (prevMagsComputed) {
        getMagnitudeSpectrogram(fftPtr, newMags.data()); // fill prevMags with magnitude values

        float flux = computeFluxValue(newMags.data(), prevMags.data());
        std::swap(prevMags, newMags);

        //retrieve amplitude value for this flux time frame window
        bool ifScanFlux = processFluxSampleAndIfScan(flux, amp);
        if (ifScanFlux) performFluxScan();
    }
    else {
        getMagnitudeSpectrogram(fftPtr, prevMags.data()); // fill prevMags with magnitude values
        prevMagsComputed = true;
    }
}

bool Graph::processFluxSampleAndIfScan(float rawFlux, float amp) {
    // Smooth this single frame
    const float smoothed = fluxSmoother.process(rawFlux);

    // Normalize with leaky-max (once per frame)
    const float normed = norm.update(smoothed);

    int64_t centerSample = (totalSamplesProcessed - 1) - (fftSize - 1) / 2;
    int64_t adjustedSample = centerSample - (int64_t)gaussDelayFrames * hopLength;

    if (adjustedSample < 0) adjustedSample = 0; // adjust to clamp during warm-up

    fluxFifo[fluxFifoIndex] = normed;
    fluxTimeFifo[fluxFifoIndex] = adjustedSample;
    ampFifo[fluxFifoIndex] = amp;

    fluxFifoIndex += 1;
    if (fluxFifoIndex == fluxSize)
        fluxFifoIndex = 0;
    
    fluxCount += 1;
    if (fluxCount == fluxHopLength) {
        fluxCount = 0;
        //make sure we've saturated our fluxFifo (only guards against the very start)
        if (totalSamplesProcessed > fluxSize * hopLength) return true;
    }
    return false;
}

void Graph::performFluxScan() {
    //Make fluxData [0... fluxSize - 1] of flux values, similar for fluxTimeData 
    copyFluxFifoToData();

    int onsetInit = 0;

    for (int i = 1; i < fluxSize - 1; i++) {
        // Track when we drop below threshold (potential onset start)
        if (fluxData[i] < metaData.onsetThresh) onsetInit = i;

        if (!havePending) {
            // Ensure enough time since last onset
            if (fluxTimeData[i] - lastOnsetCooldownAnchor > metaData.minSamplesBetweenNotes) {
                // Local peak detection
                if (fluxData[i] > fluxData[i - 1] && fluxData[i] >= fluxData[i + 1] && fluxData[i] > metaData.onsetThresh) {
                    const int64_t onsetSample = fluxTimeData[onsetInit];
                    //ensure we haven't pushed this articulation previously
                    if (!foundOnsetSamples.count(onsetSample)) {
                        pending = {}; //reset this
                        pending.onsetSample = onsetSample;
                        pending.onsetSampleIndex = onsetInit;
                        havePending = true;

                        // ArticulationWindow window;
                        // window.onsetSampleIndex = onsetInit;
                        // window.onsetSample = onsetSample;

                        // snapShots.push_back(window);
                    }
                }
            }
        }
        else {
            // Look ahead window 
            int end = std::min(i + (int) metaData.minFramesBetweenNotes, fluxSize);
            double avg = 0.0;
            int count = 0;
            for (int j = i; j < end; j++) {
                avg += fluxData[j];
                count++;
            }
            avg /= std::max(1, count);

            if (avg < metaData.sustainThresholdValue) {
                // Register sustain
                pending.sustainSample = fluxTimeData[i];
                pending.sustainSampleIndex = i;
                const int start = std::max(0, pending.onsetSampleIndex - detectionPaddingSize);
                const int end = std::min<int>(fluxSize, i + detectionPaddingSize);
                pending.flux.assign(fluxData.begin() + start, fluxData.begin() + end);
                pending.amps.assign(ampData.begin() + start, ampData.begin() + end);
                
                foundOnsetSamples.insert(pending.onsetSample);
                lastOnsetCooldownAnchor = pending.onsetSample; // to enforce min time between notes
                
                //write to our abstractfifo
                writeToAbstractArtEventFifo(&pending);
                havePending = false;
            }
        }
    }
}

void Graph::writeToAbstractArtEventFifo(ArticulationWindow* pending) {
    int s1, n1, s2, n2;
    abstractArtEventFifo.prepareToWrite(1, s1, n1, s2, n2);
    if (n1 > 0) snapShots[s1] = std::move(*pending);
    else if (n2 > 0) snapShots[s2] = std::move(*pending);

    abstractArtEventFifo.finishedWrite((n1 + n2) > 0 ? 1 : 0);
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

    //Put ampFifo into ampData
    const float* src = ampFifo.data();
    float* dst = ampData.data();
    std::memcpy(dst, src + fluxFifoIndex, (fluxSize - fluxFifoIndex) * sizeof(float));
    if (fluxFifoIndex > 0)
        std::memcpy(dst + fluxSize - fluxFifoIndex, src, fluxFifoIndex * sizeof(float));
}

void Graph::getMagnitudeSpectrogram(float* a_fftData, float* res) {
    for (int i = 0; i < numBins; i++) 
        res[i] = a_fftData[i];
}

float Graph::computeFluxValue(float* cur, float* prev) {
    if (!((int)prevMags.size() == numBins && (int)newMags.size() == numBins)) {
        DBG("Error: prevMags.size is " << prevMags.size() << " while newMags.size is " << prevMags.size());
        for (auto& i : prevMags) DBG("we have " << i << " in prevMag");
        for (auto& i : newMags) DBG("we have " << i << " in newMag");
        jassertfalse;
    }

    float ret = 0;
    for (int i = 0; i < numBins; i++)
        ret += std::max(cur[i] - prev[i], 0.0f); //half wave rectify
    return ret;
}

void Graph::valueTreePropertyChanged(juce::ValueTree& tree, const juce::Identifier& property)
{
    if (tree == scoreState) {
        if (property == tempo) 
            reset();
        
        if (property == isAnalyzing) 
            reset(); 
    }
}

void Graph::resized() {

}

void Graph::prepareToPlay(int samplesPerBlockExpected, double sr)
{
    //probably read in precomputed target recording data here
    this->sampleRate = sr;
    secondsPerSample = 1.0 / sampleRate;
    norm.sampleRate = sampleRate;
    norm.hopLength = hopLength;

    reset();

    ////initialize frequency bins:
    //double binSpacing = (double) sampleRate / fftSize;
    //for (int i = 0; i < fftSize; i++) 
    //    freqBins[i] = i * binSpacing;

    loadTargetPack(); //loads binary file for target articulation data
}

void Graph::updateMetaData() {
    ExerciseDataStruct data = ExerciseData[exerciseDataIndex + (int)scoreState.getProperty(tempo)]; //update metaData if we switch to faster or slower tempo
    metaData.onsetThresh = data.onset_thresh;
    metaData.bpm = data.bpm;
    metaData.sustainThresholdValue = data.sustain_thresh;
    metaData.minSecondsBetweenNotes = data.min_time_between;
    metaData.minSamplesBetweenNotes = static_cast<int64_t>(metaData.minSecondsBetweenNotes * sampleRate);
    metaData.minFramesBetweenNotes = static_cast<int>((metaData.minSamplesBetweenNotes + hopLength / 2) / hopLength);
}

void Graph::reset() {
    fifoIndex = 0;
    count = 0;
    totalSamplesProcessed = 0;
    std::fill(fifo.begin(), fifo.end(), 0.0f);
    std::fill(fftData.begin(), fftData.end(), 0.0f);
    fluxSmoother.reset();
    prevMagsComputed = false;
    std::fill(prevMags.begin(), prevMags.end(), 0.0f);
    std::fill(newMags.begin(), newMags.end(), 0.0f);

    fluxFifoIndex = 0;
    fluxCount = 0; 
    std::fill(fluxFifo.begin(), fluxFifo.end(), 0.0f);
    std::fill(ampFifo.begin(), ampFifo.end(), 0.0f);
    std::fill(fluxTimeData.begin(), fluxTimeData.end(), 0.0);
    std::fill(fluxData.begin(), fluxData.end(), 0.0);
    std::fill(fluxTimeFifo.begin(), fluxTimeFifo.end(), 0.0);
    std::fill(fluxTimeData.begin(), fluxTimeData.end(), 0.0);
    std::fill(ampData.begin(), ampData.end(), 0.0);
    updateMetaData();
    fluxSmoother.reset();
    foundOnsetSamples.clear();
    norm.reset();

    abstractArtEventFifo.reset();
    havePending = false;
    lastOnsetCooldownAnchor = -1;
    pending = {};
    for (auto& w : snapShots) w = ArticulationWindow{};
    if (snapShots.size() != abstractFifoCapacity) snapShots.resize(abstractFifoCapacity);
}

void Graph::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//note this must be called after `prepareToPlay`, since it needs sampleRate to be known
void Graph::loadTargetPack() {
    juce::File binFolder = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("Resources/precomputed_target_bin");
    if (!binFolder.isDirectory()) {
        DBG("ERROR: CAN;T FIND BINARY FILES AT " << binFolder.getFullPathName());
        jassert(false);
    }

    std::vector<targetpack::TargetArticulation> temp;
    if (auto res = targetpack::loadExercise(binFolder, exerciseDataIndex, (double)sampleRate, temp);
        res.wasOk())
    {
        targetArticulations.clear();
        targetArticulations.reserve(temp.size());
        for (auto& t : temp)
        {
            ArticulationWindow w;
            w.onsetSample = t.onsetSample;
            w.sustainSample = t.sustainSample;
            w.amps = std::move(t.amps);
            w.cents = std::move(t.cents);
            targetArticulations.push_back(std::move(w));
        }
    }
    else
    {
        DBG("Target pack load failed: " + res.getErrorMessage());
        targetArticulations.clear();
    }
}