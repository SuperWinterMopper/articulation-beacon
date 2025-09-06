#pragma once
#include <JuceHeader.h>
#include <vector>
#include <cstdint>

namespace targetpack
{
    struct TargetArticulation
    {
        int64_t onsetSample = 0;   // derived from onset_time (sec) * sampleRate
        int64_t sustainSample = 0;   // derived from sustain_time (sec) * sampleRate
        std::vector<float> amps;     // slice from pack.amps.f32
        std::vector<float> cents;    // slice from pack.cents.f32 (spectral centroids)
    };

    juce::Result loadExercise(const juce::File& packDir,
        int               exerciseIndex,
        double            sampleRate,
        std::vector<TargetArticulation>& out);
}