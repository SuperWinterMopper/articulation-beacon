#include "TargetPack.h"

namespace targetpack
{
    // Robust float-slice reader by element offset/length (little-endian float32)
    static bool readFloatSlice(juce::FileInputStream& stream,
        int64_t floatOffset,
        int64_t floatLen,
        std::vector<float>& out)
    {
        if (!stream.openedOk() || floatOffset < 0 || floatLen <= 0)
            return false;

        const int64_t bytePos = floatOffset * (int64_t)sizeof(float);
        const int64_t byteSize = floatLen * (int64_t)sizeof(float);
        const int64_t total = stream.getTotalLength();
        if (total < 0 || bytePos < 0 || byteSize <= 0 || (bytePos + byteSize) > total)
            return false;

        if (!stream.setPosition(bytePos))
            return false;

        out.resize((size_t)floatLen);
        auto* dst = reinterpret_cast<char*>(out.data());
        int64_t remaining = byteSize;

        while (remaining > 0)
        {
            const int want = (int)juce::jmin<int64_t>(remaining, 1 << 20); // up to 1MB
            const int got = stream.read(dst, want);
            if (got <= 0) return false;
            dst += got;
            remaining -= got;
        }
        return true;
    }

    static inline int64_t asI64(const juce::var& v)
    {
        // JSON numbers are doubles in JUCE; round to nearest integer.
        return (int64_t)juce::roundToInt64((double)v);
    }

    juce::Result loadExercise(const juce::File& packDir,
        int               exerciseIndex,
        double            sampleRate,
        std::vector<TargetArticulation>& out)
    {
        // Load and parse manifest
        const juce::File manifestFile = packDir.getChildFile("pack.json");
        if (!manifestFile.existsAsFile())
            return juce::Result::fail("pack.json not found in: " + packDir.getFullPathName());

        const juce::String jsonText = manifestFile.loadFileAsString();
        juce::var manifest = juce::JSON::parse(jsonText);
        if (manifest.isVoid() || !manifest.isObject())
            return juce::Result::fail("Invalid pack.json (not an object)");

        auto* m = manifest.getDynamicObject();
        if (m == nullptr)
            return juce::Result::fail("Invalid pack.json (no dynamic object)");

        // Resolve files for amps/cents
        juce::String ampsRel, centsRel;
        if (auto ampsVar = m->getProperty("amps"); ampsVar.isObject())
            if (auto* a = ampsVar.getDynamicObject()) ampsRel = a->getProperty("file").toString();

        if (auto centsVar = m->getProperty("cents"); centsVar.isObject())
            if (auto* c = centsVar.getDynamicObject()) centsRel = c->getProperty("file").toString();

        if (ampsRel.isEmpty() || centsRel.isEmpty())
            return juce::Result::fail("pack.json missing amps/cents file names");

        juce::FileInputStream ampsStream(packDir.getChildFile(ampsRel));
        juce::FileInputStream centsStream(packDir.getChildFile(centsRel));
        if (!ampsStream.openedOk() || !centsStream.openedOk())
            return juce::Result::fail("Failed to open pack .f32 files");

        // Select exercise
        auto exsVar = m->getProperty("exercises");
        if (!exsVar.isArray())
            return juce::Result::fail("pack.json: 'exercises' is not an array");

        auto* exs = exsVar.getArray();
        if (exerciseIndex < 0 || exerciseIndex >= exs->size())
            return juce::Result::fail("exerciseIndex out of range");

        auto exVar = (*exs)[exerciseIndex];
        if (!exVar.isObject())
            return juce::Result::fail("exercise entry is not an object");

        auto* ex = exVar.getDynamicObject();
        auto artsVar = ex->getProperty("articulations");
        if (!artsVar.isArray())
            return juce::Result::fail("exercise.articulations is not an array");

        auto* arts = artsVar.getArray();
        out.clear();
        out.reserve((size_t)arts->size());

        for (const auto& aVar : *arts)
        {
            if (!aVar.isObject()) continue;
            auto* a = aVar.getDynamicObject();

            const double onsetSec = (double)a->getProperty("onset_time");
            const double sustainSec = (double)a->getProperty("sustain_time");
            const int64_t ampOff = asI64(a->getProperty("amp_off"));
            const int64_t ampLen = asI64(a->getProperty("amp_len"));
            const int64_t centOff = asI64(a->getProperty("cent_off"));
            const int64_t centLen = asI64(a->getProperty("cent_len"));

            TargetArticulation t{};
            t.onsetSample = (int64_t)std::llround(onsetSec * sampleRate);
            t.sustainSample = (int64_t)std::llround(sustainSec * sampleRate);

            if (!readFloatSlice(ampsStream, ampOff, ampLen, t.amps))
                return juce::Result::fail("Failed reading amps slice");
            if (!readFloatSlice(centsStream, centOff, centLen, t.cents))
                return juce::Result::fail("Failed reading cents slice");

            if (t.amps.size() != t.cents.size())
            {
                const auto n = (size_t)juce::jmin(t.amps.size(), t.cents.size());
                t.amps.resize(n);
                t.cents.resize(n);
            }

            out.push_back(std::move(t));
        }

        return juce::Result::ok();
    }
} // namespace targetpack
