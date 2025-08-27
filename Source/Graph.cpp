#include <JuceHeader.h>
#include "Graph.h"

Graph::Graph(juce::ValueTree scoreState, int exerciseDataIndex) : 
    fft(fftOrder), 
    scoreState(scoreState), 
    exerciseDataIndex(exerciseDataIndex), 
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
    for (auto& i : foundOnsetSamples) {
        if (!renderedSnapShots.count(i)) { //if we have yet to render it
            renderSnapShotGraph(i);
            repaint(); //this is to show the potentially rendered graph
        }

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
                    //ensure we haven't pushed this articulation previously
                    if (!foundOnsetSamples.count(fluxTimeData[onsetInit])) {
                        ArticulationWindow window;
                        window.onsetSampleIndex = onsetInit;
                        window.onsetSample = fluxTimeData[onsetInit];

                        snapShots.push_back(window);
                        searchSustain = true;
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
                if (!snapShots.empty()) {
                    snapShots.back().sustainSampleIndex = i;
                    snapShots.back().sustainSample = fluxTimeData[i];

                    //Copy the flux and amp values to this snapShot
                    const int start = std::max(0, snapShots.back().onsetSampleIndex - detectionPaddingSize);
                    const int end = std::min<int>(fluxSize, i + detectionPaddingSize);
                    snapShots.back().flux.assign(fluxData.begin() + start, fluxData.begin() + end);
                    snapShots.back().amps.assign(ampData.begin() + start, ampData.begin() + end);
                    
                    foundOnsetSamples.insert(snapShots.back().onsetSample);
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

    //Put ampFifo into ampData
    const float* src = ampFifo.data();
    float* dst = ampData.data();
    std::memcpy(dst, src + fluxFifoIndex, (fluxSize - fluxFifoIndex) * sizeof(float));
    if (fluxFifoIndex > 0)
        std::memcpy(dst + fluxSize - fluxFifoIndex, src, fluxFifoIndex * sizeof(float));
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

void Graph::renderSnapShotGraph(int64_t onsetSample) {
    ArticulationWindow* snapShot = nullptr; //the snapShot to render's index in snapShots
    for (auto& i : snapShots) {
        if (i.onsetSample == onsetSample)
            snapShot = &i;
    }
    jassert(snapShot != nullptr);

    //to do: render graph of user articulation + match to closest in target recording
    //for now let's just make sure note onset detection works
    DBG("***************** NOTE ONSET DETECTED. GRAPH WILL DISPLAY FOR THAT ARTICULATION. ***********************");
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
    sampleRate = sr;
    secondsPerSample = 1.0 / sampleRate;
    norm.sampleRate = sampleRate;
    norm.hopLength = hopLength;

    reset();

    ////initialize frequency bins:
    //double binSpacing = (double) sampleRate / fftSize;
    //for (int i = 0; i < fftSize; i++) 
    //    freqBins[i] = i * binSpacing;

    juce::File binFolder = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("precomputed_target_bin");
    if (!binFolder.existsAsFile()) 
        jassert(false);

    (void)loadTargetPack(binFolder, exerciseDataIndex);
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
    std::fill(fftData.begin(), fftData.end(), 0.0f);
    fluxSmoother.reset();
    prevMagsComputed = false;
    std::fill(prevMags.begin(), prevMags.end(), 0.0f);
    std::fill(newMags.begin(), newMags.end(), 0.0f);
    fluxFifoIndex = 0;
    fluxCount = 0; 
    updateMetaData();
    fluxSmoother.reset();
    snapShots.clear();
    foundOnsetSamples.clear();
    renderedSnapShots.clear();
    norm.reset();
}

void Graph::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

// Helper: robustly read little-endian float32 slice by element offset/length
bool Graph::readFloatSlice(juce::FileInputStream& s,
    int64_t floatOffset,
    int64_t floatLen,
    std::vector<float>& out)
{
    if (!s.openedOk() || floatOffset < 0 || floatLen <= 0)
        return false;

    const int64_t bytePos = floatOffset * (int64_t)sizeof(float);
    const int64_t byteSize = floatLen * (int64_t)sizeof(float);
    const int64_t totalLen = s.getTotalLength();

    if (bytePos < 0 || byteSize <= 0 || bytePos + byteSize > totalLen)
        return false;

    if (!s.setPosition(bytePos))
        return false;

    out.resize((size_t)floatLen);

    // Read in a loop; InputStream::read can return fewer bytes than requested.
    auto* dst = reinterpret_cast<char*>(out.data());
    int64_t remaining = byteSize;
    while (remaining > 0)
    {
        const int chunkReq = (int)juce::jmin<int64_t>(remaining, 1 << 20); // up to 1MB chunks
        const int got = s.read(dst, chunkReq);
        if (got <= 0) return false;
        dst += got;
        remaining -= got;
    }

    // Files were written as little-endian float32 by Python.
    // On x86/arm64 (LE) this is native; add byteswap here only if you ever target big-endian.
    return true;
}

static inline int64_t asI64(const juce::var& v)
{
    // Numbers from juce::JSON::parse are doubles; round to nearest and cast.
    return (int64_t)juce::roundToInt((double)v);
}

bool Graph::loadTargetPack(const juce::File& packDir, int exerciseIndex)
{
    const juce::File manifestFile = packDir.getChildFile("pack.json");
    if (!manifestFile.existsAsFile())
    {
        DBG("pack.json not found: " + packDir.getFullPathName());
        return false;
    }

    const juce::String jsonText = manifestFile.loadFileAsString();
    juce::var manifest = juce::JSON::parse(jsonText);
    if (manifest.isVoid() || !manifest.isObject())
    {
        DBG("Invalid pack.json");
        return false;
    }
    auto* m = manifest.getDynamicObject();
    if (m == nullptr) return false;

    // Resolve binary names and (optional) dtype
    juce::String ampsRel, centsRel, ampsDType, centsDType;
    if (auto ampsVar = m->getProperty("amps"); ampsVar.isObject())
    {
        if (auto* a = ampsVar.getDynamicObject())
        {
            ampsRel = a->getProperty("file").toString();
            ampsDType = a->getProperty("dtype").toString();
        }
    }
    if (auto centsVar = m->getProperty("cents"); centsVar.isObject())
    {
        if (auto* c = centsVar.getDynamicObject())
        {
            centsRel = c->getProperty("file").toString();
            centsDType = c->getProperty("dtype").toString();
        }
    }
    if (ampsRel.isEmpty() || centsRel.isEmpty())
    {
        DBG("pack.json missing amps/cents file names");
        return false;
    }
    // Optional: sanity check dtype
    if (!ampsDType.isEmpty() && ampsDType != "float32") DBG("Warning: amps dtype != float32");
    if (!centsDType.isEmpty() && centsDType != "float32") DBG("Warning: cents dtype != float32");

    // Open streams once; we’ll seek for each slice
    juce::FileInputStream ampsStream(packDir.getChildFile(ampsRel));
    juce::FileInputStream centsStream(packDir.getChildFile(centsRel));
    if (!ampsStream.openedOk() || !centsStream.openedOk())
    {
        DBG("Failed to open .f32 files");
        return false;
    }

    // Pick exercise
    auto exsVar = m->getProperty("exercises");
    if (!exsVar.isArray()) { DBG("pack.json: exercises is not an array"); return false; }
    auto* exs = exsVar.getArray();
    if (exerciseIndex < 0 || exerciseIndex >= exs->size())
    {
        DBG("exerciseIndex out of range");
        return false;
    }

    auto exVar = (*exs)[exerciseIndex];
    if (!exVar.isObject()) { DBG("exercise entry not an object"); return false; }
    auto* ex = exVar.getDynamicObject();

    auto artsVar = ex->getProperty("articulations");
    if (!artsVar.isArray()) { DBG("exercise.articulations is not an array"); return false; }
    auto* arts = artsVar.getArray();

    targetArticulations.clear();
    targetArticulations.reserve((size_t)arts->size());

    for (const auto& aVar : *arts)
    {
        if (!aVar.isObject()) continue;
        auto* a = aVar.getDynamicObject();

        const double onsetSec = (double)a->getProperty("onset_time");
        const double sustainSec = (double)a->getProperty("sustain_time");
        const int64_t  ampOff = asI64(a->getProperty("amp_off"));
        const int64_t  ampLen = asI64(a->getProperty("amp_len"));
        const int64_t  centOff = asI64(a->getProperty("cent_off"));
        const int64_t  centLen = asI64(a->getProperty("cent_len"));

        ArticulationWindow w{};
        // sampleRate must already be set in prepareToPlay
        w.onsetSample = (int64_t)std::llround(onsetSec * (double)sampleRate);
        w.sustainSample = (int64_t)std::llround(sustainSec * (double)sampleRate);
        w.onsetSampleIndex = 0;
        w.sustainSampleIndex = 0;

        if (!readFloatSlice(ampsStream, ampOff, ampLen, w.amps)) { DBG("Failed reading amp slice");  return false; }
        if (!readFloatSlice(centsStream, centOff, centLen, w.cents)) { DBG("Failed reading cent slice"); return false; }

        // If lengths ever differ, clamp to the shorter (defensive)
        if (w.amps.size() != w.cents.size())
        {
            const auto n = (size_t)juce::jmin(w.amps.size(), w.cents.size());
            w.amps.resize(n);
            w.cents.resize(n);
        }

        targetArticulations.push_back(std::move(w));
    }

    DBG("Loaded target articulations: " + juce::String((int)targetArticulations.size()));
    return true;
}
