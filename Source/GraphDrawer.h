#pragma once

#include <JuceHeader.h>
#include "utils.h"
#include "Graph.h"

class GraphDrawer : public juce::Component
{
public:
    GraphDrawer (const ArticulationWindow& user,
                 const ArticulationWindow& target,
                 double sampleRateHz)
        : user_(user), target_(target), sr_(sampleRateHz)
    {
        setOpaque (true);

        // Replace with your assets (or embed via BinaryData).
        startIcon_ = juce::ImageFileFormat::loadFrom (juce::File ("C:/temp/start_icon.png"));
        faceIcon_  = juce::ImageFileFormat::loadFrom (juce::File ("C:/temp/face_icon.png"));
    }

    void setData (const ArticulationWindow& user,
                  const ArticulationWindow& target,
                  double sampleRateHz)
    {
        user_ = user; target_ = target; sr_ = sampleRateHz;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::white);

        // --- Layout ---
        auto outer  = getLocalBounds().toFloat().reduced (10.0f);
        auto topBar = outer.removeFromTop (28.0f);
        auto xAxis  = outer.removeFromBottom (24.0f);
        auto plot   = outer.reduced (6.0f);

        // Title + face icon (placeholder)
        g.setColour (juce::Colours::black.withAlpha (0.88f));
        g.setFont (juce::Font (15.0f, juce::Font::bold));
        g.drawText ("Articulation Graph", topBar.toNearestInt(), juce::Justification::centredLeft);

        if (faceIcon_.isValid())
        {
            auto face = juce::Rectangle<float> (24.0f, 24.0f)
                        .withCentre ({ plot.getCentreX(), topBar.getCentreY() });
            g.drawImageWithin (faceIcon_, (int) face.getX(), (int) face.getY(),
                               (int) face.getWidth(), (int) face.getHeight(),
                               juce::RectanglePlacement::centred);
        }

        const int Nu = (int) user_.amps.size();
        const int Nt = (int) target_.amps.size();

        // Draw axes if nothing usable
        if (Nu == 0 && Nt == 0)
        {
            drawAxes (g, plot, xAxis, 1.0 /* seconds */);
            return;
        }

        // --- Safe sample rate ---
        const double ampsRateHz = 44800 / 512.0;   // effective rate for per-hop arrays

        // --- Per-series start/end times (seconds) using onsetSample ---
        const double tU0 = (Nu > 0 ? (double) user_.onsetSample  / sr_ : 0.0);
        const double tT0 = (Nt > 0 ? (double) target_.onsetSample / sr_ : 0.0);
        const double dU = (Nu > 0 ? (double)Nu / ampsRateHz : 0.0);
        const double dT = (Nt > 0 ? (double)Nt / ampsRateHz : 0.0);
        const double tU1 = tU0 + dU;
        const double tT1 = tT0 + dT;

        // Global time window: from earliest start to latest end
        double tStart = std::min (tU0, tT0);
        double tEnd   = std::max (tU1, tT1);
        double span = tEnd - tStart;
        if (Nu > 0 && Nt == 0) {
            tStart = tU0;
            tEnd = tU1;
            span = tEnd - tStart;
        }
        else if (Nt > 0 && Nu == 0) {
            tStart = tT0;
            tEnd = tT1;
            span = tEnd - tStart;
        }

        if (!(span > 0.0)) span = 1.0; // degenerate guard

        // --- Ranges (skip non-finite) ---
        const auto [amin, amax] = minMaxFinite (user_.amps, target_.amps, 0.0f, 1.0f);
        const auto [cmin, cmax] = minMaxFinite (user_.cents, target_.cents, 0.0f, 1.0f);

        // --- Discretization ---
        const int N = juce::jlimit (120, 1200, (int) std::round (plot.getWidth()));
        if (N < 2)
        {
            drawAxes (g, plot, xAxis, span);
            return;
        }

        tmpX_.assign (N, 0.0f);
        yU_.assign (N, 0.0f);
        yT_.assign (N, 0.0f);
        cU_.assign (N, (float) cmin);
        cT_.assign (N, (float) cmin);
        presentU_.assign (N, 0);
        presentT_.assign (N, 0);

        auto mapX = [&] (double absTime)
        {
            const double f = juce::jlimit (0.0, 1.0, (absTime - tStart) / span);
            return plot.getX() + (float) (f * plot.getWidth());
        };
        auto mapY = [&] (float amp)
        {
            return juce::jmap (amp, (float) amin, (float) amax, plot.getBottom(), plot.getY());
        };
        auto lerp = [] (float a, float b, float t) { return a + (b - a) * t; };

        auto atLinear = [&] (const std::vector<float>& v, double idx)
        {
            if (v.empty()) return 0.0f;
            if (!(idx > 0.0)) return v.front();
            const double last = (double) v.size() - 1.0;
            if (!(idx < last)) return v.back();

            const int i0 = (int) std::floor (idx);
            const int i1 = i0 + 1;
            const float t = (float) (idx - (double) i0);

            const float a = v[(size_t) i0];
            const float b = v[(size_t) i1];
            // Guard against NaN
            const float aa = (std::isfinite (a) ? a : 0.0f);
            const float bb = (std::isfinite (b) ? b : aa);
            return lerp (aa, bb, t);
        };

        struct SeriesView
        {
            const std::vector<float>& amps;
            const std::vector<float>& cents;
            double t0 = 0.0, t1 = 0.0;
            bool valid() const { return !amps.empty() && t1 > t0; }
        };

        SeriesView U { user_.amps,   user_.cents,   tU0, tU1 };
        SeriesView T { target_.amps, target_.cents, tT0, tT1 };

        auto sampleAt = [&] (const SeriesView& S, double tAbs, float& ampOut, float& centOut, uint8_t& present)
        {
            if (! S.valid() || tAbs < S.t0 || tAbs > S.t1)
            {
                ampOut = 0.0f; centOut = (float) cmin; present = 0; return;
            }

            const double tRel = tAbs - S.t0;
            const double dur  = S.t1  - S.t0;
            const double ampsRateHz = sr_ / 512.0; // GraphDrawer includes Graph.h
            const double idxA = tRel * ampsRateHz;       // use the variable defined above
            // cents spread evenly across duration (size may differ)
            const double idxC = S.cents.empty() ? 0.0
                                 : (tRel / std::max (dur, 1e-9)) * std::max (1.0, (double) S.cents.size() - 1.0);

            ampOut  = atLinear (S.amps,  idxA);
            centOut = S.cents.empty() ? (float) cmin : atLinear (S.cents, idxC);

            // Replace non-finite with 0 to avoid bad draws
            if (!std::isfinite (ampOut))  ampOut  = 0.0f;
            if (!std::isfinite (centOut)) centOut = (float) cmin;

            present = 1;
        };

        // Sample across the absolute time span
        for (int i = 0; i < N; ++i)
        {
            const double f = (double) i / (double) (N - 1);
            const double tAbs = tStart + f * span;

            tmpX_[i] = mapX (tAbs);

            sampleAt (U, tAbs, yU_[i], cU_[i], presentU_[i]);
            sampleAt (T, tAbs, yT_[i], cT_[i], presentT_[i]);
        }

        // --- Shaded areas: fill for each contiguous overlap [i0..i1] ---
        auto fillOverlaps = [&]()
        {
            int i = 0;
            while (i < N)
            {
                // find start of overlap
                while (i < N && !(presentU_[i] && presentT_[i])) ++i;
                if (i >= N) break;
                const int i0 = i;
                // find end of overlap
                while (i < N && presentU_[i] && presentT_[i]) ++i;
                const int i1 = i - 1;
                if (i1 <= i0) continue;

                juce::Path band;
                band.startNewSubPath (tmpX_[i0], mapY (yU_[i0]));
                for (int k = i0 + 1; k <= i1; ++k)
                    band.lineTo (tmpX_[k], mapY (yU_[k]));
                for (int k = i1; k >= i0; --k)
                    band.lineTo (tmpX_[k], mapY (yT_[k]));
                band.closeSubPath();

                g.setColour (juce::Colours::red.withAlpha (0.12f));
                g.fillPath (band);
            }
        };
        fillOverlaps();

        // --- Draw series with grey→yellow by centroid; respect presence ---
        auto drawSeries = [&] (const std::vector<float>& Y,
                               const std::vector<float>& C,
                               const std::vector<uint8_t>& present,
                               float alpha)
        {
            const juce::Colour baseGrey = juce::Colours::grey;
            const juce::Colour yellow   = juce::Colours::yellow;

            auto normC = [&] (float v)
            {
                const float denom = (float) (cmax - cmin);
                const float t = (denom > 0.0f ? (v - (float) cmin) / denom : 0.0f);
                return juce::jlimit (0.0f, 1.0f, t);
            };

            for (int i = 1; i < N; ++i)
            {
                if (!(present[i - 1] && present[i])) continue;

                const float x0 = tmpX_[i - 1], y0 = mapY (Y[i - 1]);
                const float x1 = tmpX_[i],     y1 = mapY (Y[i]);

                const float tC = normC (0.5f * (C[i - 1] + C[i]));
                const juce::Colour col = baseGrey.interpolatedWith (yellow, tC).withAlpha (alpha);

                g.setColour (col);
                g.drawLine (x0, y0, x1, y1, 2.0f);
            }
        };

        // Target first (lighter), then User (stronger)
        drawSeries (yT_, cT_, presentT_, 0.35f);
        drawSeries (yU_, cU_, presentU_, 1.00f);

        // --- Start markers (exact start times) ---
        auto drawStartIcon = [&] (const SeriesView& S)
        {
            if (!startIcon_.isValid() || !S.valid()) return;

            // Get amplitude exactly at start
            float amp = 0.0f, cen = (float) cmin; uint8_t pres = 0;
            sampleAt (S, S.t0, amp, cen, pres);

            const float x = mapX (S.t0);
            const float y = mapY (amp);
            auto R = juce::Rectangle<float> (12.0f, 12.0f).withCentre ({ x, y });

            g.drawImageWithin (startIcon_, (int) R.getX(), (int) R.getY(),
                               (int) R.getWidth(), (int) R.getHeight(),
                               juce::RectanglePlacement::centred);
        };
        drawStartIcon (T);
        drawStartIcon (U);

        // --- Axes + X ticks (seconds) ---
        drawAxes (g, plot, xAxis, span);

        DBG("Nu=" << Nu << " Nt=" << Nt
            << " tU0=" << tU0 << " tU1=" << tU1
            << " tT0=" << tT0 << " tT1=" << tT1
            << " N=" << N);

        int cntU = std::accumulate(presentU_.begin(), presentU_.end(), 0);
        int cntT = std::accumulate(presentT_.begin(), presentT_.end(), 0);
        DBG("presentU count=" << cntU << " presentT count=" << cntT);
    }

private:
    // Data
    ArticulationWindow user_, target_;
    double sr_ = 48000.0;

    // Images
    juce::Image startIcon_;
    juce::Image faceIcon_;

    // Scratch
    std::vector<float>    yU_, yT_, cU_, cT_, tmpX_;
    std::vector<uint8_t>  presentU_, presentT_;

    // Helpers ----------------------------------------------------------

    static std::pair<float, float> minMaxFinite (const std::vector<float>& a,
                                                 const std::vector<float>& b,
                                                 float fallbackMin,
                                                 float fallbackMax)
    {
        float mn =  std::numeric_limits<float>::infinity();
        float mx = -std::numeric_limits<float>::infinity();

        auto scan = [&] (const std::vector<float>& v)
        {
            for (float x : v)
            {
                if (!std::isfinite (x)) continue;
                mn = std::min (mn, x);
                mx = std::max (mx, x);
            }
        };
        scan (a); scan (b);

        if (!std::isfinite (mn) || !std::isfinite (mx) || mx <= mn)
            return { fallbackMin, fallbackMax };

        return { mn, mx };
    }

    static juce::String secs (double s)
    {
        // 1 decimal place, e.g. 0.0s, 0.5s, 2.3s
        return juce::String (s, 1) + "s";
    }

    void drawAxes (juce::Graphics& g, juce::Rectangle<float> plot, juce::Rectangle<float> xAxis, double spanSeconds)
    {
        // Plot border
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawRect (plot);

        // 5 vertical grid lines + labels: 0, 0.25, 0.5, 0.75, 1 span
        g.setFont (12.0f);
        for (int i = 0; i <= 4; ++i)
        {
            const float f = (float) i / 4.0f;
            const float x = plot.getX() + f * plot.getWidth();

            g.setColour (juce::Colours::grey.withAlpha (0.35f));
            g.drawVerticalLine ((int) std::round (x), plot.getY(), plot.getBottom());

            g.setColour (juce::Colours::black.withAlpha (0.75f));
            g.drawText (secs ((double) f * spanSeconds),
                        juce::Rectangle<int> ((int) x - 24, (int) xAxis.getY(), 48, (int) xAxis.getHeight()),
                        juce::Justification::centredTop);
        }
    }
};
