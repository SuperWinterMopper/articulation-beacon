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

    juce::File faceIconFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("Resources/smiley.png");
    juce::File startIconFile = juce::File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory().getChildFile("Resources/downarrow.png");


    faceIcon_  = juce::ImageFileFormat::loadFrom (juce::File ("C:/Users/afkhe/Downloads/smiley.png"));
    startIcon_ = juce::ImageFileFormat::loadFrom (juce::File ("C:/Users/afkhe/Downloads/downarrow.png"));
    }

    void setData (const ArticulationWindow& user, const ArticulationWindow& target, double sampleRateHz) {
        user_ = user; target_ = target; sr_ = sampleRateHz;
        repaint();
    }

    void paint (juce::Graphics& g) override {
        g.fillAll (juce::Colours::white);

        auto outer  = getLocalBounds().toFloat().reduced (10.0f);
        auto topBar = outer.removeFromTop (28.0f);
        auto xAxis  = outer.removeFromBottom (24.0f);
        auto plot   = outer.reduced (6.0f);

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

        if (Nu == 0 && Nt == 0)
        {
            drawAxes (g, plot, xAxis, /*tStart*/ 0.0, /*span*/ 1.0);
            DBG("GraphDrawer::paint: no data to plot");
            return;
        }

    // --- Safe sample rate & per-hop rate (fixed hop for visualization per request) ---
    const double sr = (sr_ > 0.0 ? sr_ : 48000.0);
    const double ampsRateHz = sr / 512.0; // DO NOT change to Graph::hopLength (user request)
    DBG("GraphDrawer::paint: sr=" << sr << " fixedHop=512 ampsRateHz=" << ampsRateHz);

        // --- Per-series start/end times (seconds) using onsetSample ---
        const double tU0 = (Nu > 0 ? (double) user_.onsetSample  / sr_ : 0.0);
        const double tT0 = (Nt > 0 ? (double) target_.onsetSample / sr_ : 0.0);
        const double dU  = (Nu > 0 ? (double) Nu / ampsRateHz : 0.0);
        const double dT  = (Nt > 0 ? (double) Nt / ampsRateHz : 0.0);
        const double tU1 = tU0 + dU;
        const double tT1 = tT0 + dT;

        {
            static constexpr bool kAlignEndsMode = true; // toggle if needed
            if (kAlignEndsMode)
            {
                // Truncate target to first 1/10th of its samples for visualization (shape focus)
                size_t NtFull = target_.amps.size();
                size_t NtSlice = (NtFull > 0 ? std::max<size_t>(1, NtFull / 10) : 0);
                std::vector<float> targetAmpsSlice; targetAmpsSlice.reserve(NtSlice);
                for (size_t i = 0; i < NtSlice; ++i) targetAmpsSlice.push_back(target_.amps[i]);
                // Proportionally slice cents (maintain relative coverage)
                size_t NcFull = target_.cents.size();
                size_t NcSlice = (NcFull > 0 ? std::max<size_t>(1, (size_t) std::floor((double)NcFull * (NtSlice / (double) std::max<size_t>(1, NtFull)))) : 0);
                std::vector<float> targetCentsSlice; targetCentsSlice.reserve(NcSlice);
                for (size_t i = 0; i < NcSlice; ++i) targetCentsSlice.push_back(target_.cents[i]);
                const double dT_original = dT; // keep original for debug
                const double dT = (NtSlice > 0 ? (double) NtSlice / ampsRateHz : 0.0); // truncated duration
                const double maxDur = std::max(dU, dT);
                if (maxDur <= 0.0)
                {
                    drawAxes (g, plot, xAxis, 0.0, 1.0);
                    return;
                }

                const int N = juce::jlimit (120, 1200, (int) std::round (plot.getWidth()));
                if (N < 2)
                {
                    drawAxes (g, plot, xAxis, 0.0, maxDur);
                    return;
                }

                tmpX_.assign (N, 0.0f);
                yU_.assign (N, 0.0f); yT_.assign (N, 0.0f);
                cU_.assign (N, 0.0f); cT_.assign (N, 0.0f);
                presentU_.assign (N, 0); presentT_.assign (N, 0);
                std::vector<float> progU (N, -1.0f), progT (N, -1.0f); // per-sample progress (0..1) for colour gradients

                auto lerp = [] (float a, float b, float t){ return a + (b - a) * t; };
                auto atLinearLocal = [&] (const std::vector<float>& v, double idx)
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
                    const float aa = (std::isfinite(a) ? a : 0.0f);
                    const float bb = (std::isfinite(b) ? b : aa);
                    return lerp (aa, bb, t);
                };

                double displayDurU = dU;
                if (dU > 0.0 && dU < 0.5 * maxDur)
                    displayDurU = 0.5 * maxDur;
                const double startDisplayAxisU = (dU > 0.0 ? (maxDur - displayDurU) : 0.0);
                const double warpFactorU = (dU > 0.0 ? (displayDurU / dU) : 1.0);

                double displayDurT = dT;
                if (dT > 0.0 && dT < 0.5 * maxDur)
                    displayDurT = 0.5 * maxDur; // stretch visually (non-time-accurate) for visibility
                const double startDisplayAxisT = (dT > 0.0 ? (maxDur - displayDurT) : 0.0);
                const double warpFactorT = (dT > 0.0 ? (displayDurT / dT) : 1.0); // >1 means stretched
                DBG("AlignEndsMode maxDur=" << maxDur
                    << " dU=" << dU << " dT=" << dT
                    << " userDisplayDur=" << displayDurU << " userDisplayStart=" << startDisplayAxisU << " userWarpFactor=" << warpFactorU
                    << " targetDisplayDur=" << displayDurT << " targetDisplayStart=" << startDisplayAxisT << " targetWarpFactor=" << warpFactorT);

                for (int i = 0; i < N; ++i)
                {
                    const double f = (double) i / (double) (N - 1);
                    const double tAxis = f * maxDur; // 0..maxDur
                    tmpX_[(size_t) i] = plot.getX() + (float) (f * plot.getWidth());

                    // User series sampling (with possible visual stretch)
                    if (dU > 0.0)
                    {
                        const double q = (tAxis - startDisplayAxisU) / std::max(displayDurU, 1e-9); // 0..1 over displayed (possibly stretched) duration
                        if (q >= 0.0 && q <= 1.0)
                        {
                            const double tRel = q * dU; // actual time within user series
                            const double idxA = tRel * ampsRateHz;
                            const double idxC = user_.cents.empty() ? 0.0 : (tRel / std::max(dU,1e-9)) * std::max(1.0, (double) user_.cents.size() - 1.0);
                            yU_[(size_t) i] = atLinearLocal (user_.amps, idxA);
                            cU_[(size_t) i] = user_.cents.empty() ? 0.0f : atLinearLocal (user_.cents, idxC);
                            presentU_[(size_t) i] = 1;
                            progU[(size_t) i] = (float) juce::jlimit (0.0, 1.0, q);
                        }
                    }

                    // Target series sampling (with possible visual stretch, truncated data)
                    if (dT > 0.0)
                    {
                        const double q = (tAxis - startDisplayAxisT) / std::max(displayDurT, 1e-9); // 0..1 over displayed (possibly stretched) duration
                        if (q >= 0.0 && q <= 1.0)
                        {
                            const double tRel = q * dT; // actual time within truncated target series
                            const double idxA = tRel * ampsRateHz; // index into truncated amps
                            const double idxC = targetCentsSlice.empty() ? 0.0 : (tRel / std::max(dT,1e-9)) * std::max(1.0, (double) targetCentsSlice.size() - 1.0);
                            yT_[(size_t) i] = atLinearLocal (targetAmpsSlice, idxA);
                            cT_[(size_t) i] = targetCentsSlice.empty() ? 0.0f : atLinearLocal (targetCentsSlice, idxC);
                            presentT_[(size_t) i] = 1;
                            progT[(size_t) i] = (float) juce::jlimit (0.0, 1.0, q); // progress over displayed length
                        }
                    }
                }

                auto finiteMinMax = [] (const std::vector<float>& v, float fbMin, float fbMax){
                    float mn =  std::numeric_limits<float>::infinity();
                    float mx = -std::numeric_limits<float>::infinity();
                    for (float x : v) if (std::isfinite(x)) { mn = std::min(mn,x); mx = std::max(mx,x); }
                    if (!std::isfinite(mn) || !std::isfinite(mx) || mx <= mn) { mn = fbMin; mx = fbMax; }
                    return std::pair<float,float>{mn,mx};
                };
                auto [uMin,uMax] = finiteMinMax(user_.amps, 0.0f, 1.0f);
                auto [tMin,tMax] = finiteMinMax(target_.amps, 0.0f, 1.0f);
                if (uMax - uMin < 1e-9f) { uMin -= 0.5f; uMax += 0.5f; }
                if (tMax - tMin < 1e-9f) { tMin -= 0.5f; tMax += 0.5f; }
                auto mapYUser = [&] (float a){ return juce::jmap(a, uMin, uMax, plot.getBottom(), plot.getY()); };
                auto mapYTarget = [&] (float a){ return juce::jmap(a, tMin, tMax, plot.getBottom(), plot.getY()); };

                auto drawSeries = [&] (const char* label,
                                       const std::vector<float>& Y,
                                       const std::vector<uint8_t>& present,
                                       const std::vector<float>& prog,
                                       float alpha,
                                       bool isUser)
                {
                    const juce::Colour baseGrey = juce::Colours::darkgrey;
                    const juce::Colour yellow   = juce::Colours::yellow;
                    int segs = 0; int presentCount = 0;
                    for (size_t i = 0; i < present.size(); ++i) if (present[i]) ++presentCount;
                    for (int i = 1; i < (int) present.size(); ++i)
                    {
                        if (!(present[i-1] && present[i])) continue;
                        const float y0 = isUser ? mapYUser(Y[(size_t)i-1]) : mapYTarget(Y[(size_t)i-1]);
                        const float y1 = isUser ? mapYUser(Y[(size_t)i])   : mapYTarget(Y[(size_t)i]);
                        const float x0 = tmpX_[(size_t)i-1];
                        const float x1 = tmpX_[(size_t)i];
                        // Local progress (average of endpoints)
                        float p = 0.5f * (prog[(size_t)i-1] + prog[(size_t)i]);
                        if (!(p > 0.0f)) p = prog[(size_t)i];
                        if (!(p > 0.0f)) p = 0.0f;
                        float tC = 0.0f;
                        if (isUser)
                        {
                            static constexpr float kUserEarlyStart = 0.10f;
                            static constexpr float kUserMid = 0.40f;
                            if (p <= kUserEarlyStart) {
                                tC = (p / kUserEarlyStart) * 0.25f;
                            } else if (p <= kUserMid) {
                                const float f2 = (p - kUserEarlyStart) / (kUserMid - kUserEarlyStart);
                                tC = 0.25f + f2 * (0.60f - 0.25f);
                            } else {
                                const float f2 = (p - kUserMid) / (1.0f - kUserMid);
                                tC = 0.60f + f2 * (1.0f - 0.60f);
                            }
                        }
                        else
                        {
                            static constexpr float kTargetYellowStart = 0.85f;
                            if (p <= kTargetYellowStart) tC = 0.0f; else tC = (p - kTargetYellowStart) / (1.0f - kTargetYellowStart);
                        }
                        const juce::Colour col = baseGrey.interpolatedWith (yellow, juce::jlimit (0.0f, 1.0f, tC)).withAlpha (alpha);
                        const float w = juce::jmap (juce::jlimit (0.0f, 1.0f, tC), 0.0f, 1.0f, 2.0f, 3.0f);
                        g.setColour (col);
                        g.drawLine (x0, y0, x1, y1, w);
                        ++segs;
                    }
                    DBG("AlignEnds DrawSeries " << label << " present=" << presentCount << " segs=" << segs);
                };

                drawSeries("Target", yT_, presentT_, progT, 0.55f, false);
                drawSeries("User",   yU_, presentU_, progU, 1.00f, true);

                // Start markers (using first present sample for Y)
                auto drawStartMarker = [&] (bool isUser, double startAxis, double dur, const std::vector<float>& Y, const std::vector<uint8_t>& present)
                {
                    if (!(dur > 0.0)) return;
                    float yRaw = 0.0f;
                    for (size_t i = 0; i < Y.size(); ++i) if (present[i]) { yRaw = Y[i]; break; }
                    const float x = plot.getX() + (float) ((startAxis / maxDur) * plot.getWidth());
                    const float y = (isUser ? mapYUser(yRaw) : mapYTarget(yRaw));
                    auto R = juce::Rectangle<float> (12.0f, 12.0f).withCentre ({ x, y });
                    if (startIcon_.isValid())
                        g.drawImageWithin (startIcon_, (int) R.getX(), (int) R.getY(), (int) R.getWidth(), (int) R.getHeight(), juce::RectanglePlacement::centred);
                    else
                    {
                        g.setColour(isUser ? juce::Colours::orange : juce::Colours::steelblue);
                        g.fillEllipse(R);
                        g.setColour(juce::Colours::black.withAlpha(0.6f));
                        g.drawEllipse(R, 1.2f);
                    }
                };
                drawStartMarker(true,  startDisplayAxisU, dU, yU_, presentU_);

                drawStartMarker(false, startDisplayAxisT, dT, yT_, presentT_);


                drawAxes (g, plot, xAxis, 0.0, maxDur);
                DBG("AlignEndsMode complete NtFull=" << NtFull << " NtSlice=" << NtSlice << " dT_original=" << dT_original << " dT_trunc=" << dT);
                return; 
            }
        }

        double tStart = std::min (tU0, tT0);
        double tEnd   = std::max (tU1, tT1);
        double span = tEnd - tStart;
        if (Nu > 0 && Nt == 0) { tStart = tU0; tEnd = tU1; span = tEnd - tStart; }
        else if (Nt > 0 && Nu == 0) { tStart = tT0; tEnd = tT1; span = tEnd - tStart; }

        static constexpr double kViewWindow = 0.30; // 300 ms
        {
            const bool uOK = (Nu > 0);
            const bool tOK = (Nt > 0);
            const double minStart = std::min (uOK ? tU0 : tStart, tOK ? tT0 : tStart);
            const double maxEnd   = std::max (uOK ? tU1 : tStart, tOK ? tT1 : tStart);
            if (maxEnd > minStart) {
                if (maxEnd - minStart > kViewWindow) {
                    tEnd   = maxEnd;
                    tStart = tEnd - kViewWindow;
                } else {
                    // data shorter than 300 ms -> show exact extent
                    tStart = minStart;
                    tEnd   = maxEnd;
                }
                span = tEnd - tStart;
            }
        }
        if (!(span > 0.0)) { tEnd = tStart + kViewWindow; span = kViewWindow; }
        DBG("TimeWindow tStart=" << tStart << " tEnd=" << tEnd << " span=" << span
            << " tU0=" << tU0 << " tU1=" << tU1 << " tT0=" << tT0 << " tT1=" << tT1);

        const auto [amin, amax] = minMaxFinite (user_.amps, target_.amps, 0.0f, 1.0f);
        // (C) Ensure visible dynamic range for amplitudes (prevent "flat" look)
        float a0 = amin, a1 = amax;
        if (!std::isfinite (a0) || !std::isfinite (a1) || a1 <= a0) { a0 = 0.0f; a1 = 1.0f; }
        if (a1 - a0 < 1e-6f) {
            const float mid = 0.5f * (a0 + a1);
            a0 = mid - 0.5f;
            a1 = mid + 0.5f;
        }
        DBG("AmplitudeRange raw amin=" << amin << " amax=" << amax << " -> expanded a0=" << a0 << " a1=" << a1
            << " Nu=" << Nu << " Nt=" << Nt);
        const auto [cmin, cmax] = minMaxFinite (user_.cents, target_.cents, 0.0f, 1.0f);

        // --- Discretization ---
        const int N = juce::jlimit (120, 1200, (int) std::round (plot.getWidth()));
        if (N < 2)
        {
            drawAxes (g, plot, xAxis, tStart, span);
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
            return juce::jmap (amp, a0, a1, plot.getBottom(), plot.getY());
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
        DBG("Series validity U=" << (int) U.valid() << " T=" << (int)T.valid()
            << " user amps size=" << user_.amps.size() << " target amps size=" << target_.amps.size()
            << " user cents size=" << user_.cents.size() << " target cents size=" << target_.cents.size());

        auto sampleAtAbsolute = [&] (const SeriesView& S, double tAbs, float& ampOut, float& centOut, uint8_t& present)
        {
            if (! S.valid() || tAbs < S.t0 || tAbs > S.t1) { ampOut = 0.0f; centOut = (float) cmin; present = 0; return; }
            const double tRel = tAbs - S.t0;
            const double dur  = S.t1  - S.t0;
            const double idxA = tRel * ampsRateHz;
            const double idxC = S.cents.empty() ? 0.0 : (tRel / std::max (dur, 1e-9)) * std::max (1.0, (double) S.cents.size() - 1.0);
            ampOut  = atLinear (S.amps,  idxA);
            centOut = S.cents.empty() ? (float) cmin : atLinear (S.cents, idxC);
            if (!std::isfinite (ampOut))  ampOut  = 0.0f;
            if (!std::isfinite (centOut)) centOut = (float) cmin;
            present = 1;
        };

        // Decide if we need overlay mode: series do not overlap in time -> show both normalized
        bool disjoint = false;
        if (U.valid() && T.valid())
            disjoint = (U.t1 < T.t0 || T.t1 < U.t0);
        const bool overlayMode = disjoint; // could add other heuristics
        DBG("OverlayMode(disjoint)=" << (int) overlayMode);

        if (overlayMode)
        {
            // Normalize both series to their own 0..1 timeline and resample to N points.
            for (int i = 0; i < N; ++i)
            {
                const double f = (double) i / (double) (N - 1);
                tmpX_[i] = plot.getX() + (float) (f * plot.getWidth());
                if (U.valid()) {
                    const double idxA = f * std::max (1.0, (double) U.amps.size() - 1.0);
                    yU_[i] = atLinear (U.amps, idxA);
                    const double idxC = f * std::max (1.0, (double) U.cents.size() - 1.0);
                    cU_[i] = U.cents.empty() ? (float) cmin : atLinear (U.cents, idxC);
                    presentU_[i] = 1;
                }
                if (T.valid()) {
                    const double idxA = f * std::max (1.0, (double) T.amps.size() - 1.0);
                    yT_[i] = atLinear (T.amps, idxA);
                    const double idxC = f * std::max (1.0, (double) T.cents.size() - 1.0);
                    cT_[i] = T.cents.empty() ? (float) cmin : atLinear (T.cents, idxC);
                    presentT_[i] = 1;
                }
            }
            tStart = 0.0; span = 1.0; tEnd = 1.0;
        }
        else
        {
            for (int i = 0; i < N; ++i)
            {
                const double f = (double) i / (double) (N - 1);
                const double tAbs = tStart + f * span;
                tmpX_[i] = mapX (tAbs);
                sampleAtAbsolute (U, tAbs, yU_[i], cU_[i], presentU_[i]);
                sampleAtAbsolute (T, tAbs, yT_[i], cT_[i], presentT_[i]);
            }
            // Fallback: if the user series exists but no samples landed in window, switch to overlay normalization.
            if (U.valid()) {
                int userPresentCount = std::accumulate(presentU_.begin(), presentU_.end(), 0);
                if (userPresentCount == 0) {
                    DBG("FallbackOverlay: user series had data but 0 samples in current window. Enabling overlay normalization.");
                    jassertfalse; // Should not happen in normal operation; indicates time-window exclusion.
                    for (int i = 0; i < N; ++i)
                    {
                        const double f = (double) i / (double) (N - 1);
                        tmpX_[i] = plot.getX() + (float) (f * plot.getWidth());
                        if (U.valid()) {
                            const double idxA = f * std::max (1.0, (double) U.amps.size() - 1.0);
                            yU_[i] = atLinear (U.amps, idxA);
                            const double idxC = f * std::max (1.0, (double) U.cents.size() - 1.0);
                            cU_[i] = U.cents.empty() ? (float) cmin : atLinear (U.cents, idxC);
                            presentU_[i] = 1;
                        }
                        if (T.valid()) {
                            const double idxA = f * std::max (1.0, (double) T.amps.size() - 1.0);
                            yT_[i] = atLinear (T.amps, idxA);
                            const double idxC = f * std::max (1.0, (double) T.cents.size() - 1.0);
                            cT_[i] = T.cents.empty() ? (float) cmin : atLinear (T.cents, idxC);
                            presentT_[i] = 1;
                        }
                    }
                    tStart = 0.0; span = 1.0; tEnd = 1.0; // normalized axis
                }
            }
        }

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
        if (!overlayMode)
            fillOverlaps();

        // --- Independent per-series amplitude normalization & x-gradient colouring ---
        auto finiteMinMax = [] (const std::vector<float>& v, float fallbackMin, float fallbackMax){
            float mn =  std::numeric_limits<float>::infinity();
            float mx = -std::numeric_limits<float>::infinity();
            for (float x : v) if (std::isfinite(x)) { mn = std::min(mn,x); mx = std::max(mx,x); }
            if (!std::isfinite(mn) || !std::isfinite(mx) || mx <= mn) { mn = fallbackMin; mx = fallbackMax; }
            return std::pair<float,float>{mn,mx};
        };
        auto [uMin,uMax] = finiteMinMax(user_.amps, 0.0f, 1.0f);
        auto [tMin,tMax] = finiteMinMax(target_.amps, 0.0f, 1.0f);
        if (uMax - uMin < 1e-9f) { uMin -= 0.5f; uMax += 0.5f; }
        if (tMax - tMin < 1e-9f) { tMin -= 0.5f; tMax += 0.5f; }
        DBG("PerSeriesNorm user[min,max]=" << uMin << "," << uMax << " target[min,max]=" << tMin << "," << tMax);

        auto mapYUser = [&] (float a){ return juce::jmap(a, uMin, uMax, plot.getBottom(), plot.getY()); };
        auto mapYTarget = [&] (float a){ return juce::jmap(a, tMin, tMax, plot.getBottom(), plot.getY()); };

        auto drawSeries = [&] (const char* label,
                               const std::vector<float>& Y,
                               const std::vector<uint8_t>& present,
                               float alpha,
                               bool isUser)
        {
            const juce::Colour baseGrey = juce::Colours::darkgrey;
            const juce::Colour yellow   = juce::Colours::yellow;
            int segs = 0; int presentCount = 0; float yMin= std::numeric_limits<float>::infinity(); float yMax = -yMin;
            for (int i = 0; i < N; ++i) if (present[i]) { ++presentCount; yMin = std::min(yMin, Y[i]); yMax = std::max(yMax, Y[i]); }
            if (isUser && yT_.size()==Y.size()) {
                int eq=0; for (size_t k=0;k<Y.size();++k) if (present[k] && presentT_[k] && std::abs(Y[k]-yT_[k]) < 1e-9f) ++eq; 
                DBG("User/Target identical sample count=" << eq << " of " << Y.size());
            }
            for (int i = 1; i < N; ++i)
            {
                if (!(present[i - 1] && present[i])) continue;
                const float x0 = tmpX_[i - 1];
                const float x1 = tmpX_[i];
                const float y0 = (isUser ? mapYUser(Y[i-1]) : mapYTarget(Y[i-1]));
                const float y1 = (isUser ? mapYUser(Y[i])   : mapYTarget(Y[i]));
                // Progress across width
                const float tProg = (float) i / (float) (N - 1);
                float tC = 0.0f;
                if (isUser) {
                    static constexpr float kUserEarlyStart = 0.10f;
                    static constexpr float kUserMid = 0.40f;
                    if (tProg <= kUserEarlyStart) {
                        tC = (tProg / kUserEarlyStart) * 0.25f; // up to 25% yellow
                    } else if (tProg <= kUserMid) {
                        const float f = (tProg - kUserEarlyStart) / (kUserMid - kUserEarlyStart);
                        tC = 0.25f + f * (0.60f - 0.25f); // 25% -> 60%
                    } else {
                        const float f = (tProg - kUserMid) / (1.0f - kUserMid);
                        tC = 0.60f + f * (1.0f - 0.60f); // 60% -> 100%
                    }
                } else {
                    // Target: stay grey until 85%, then ramp.
                    static constexpr float kTargetYellowStart = 0.85f;
                    if (tProg <= kTargetYellowStart) tC = 0.0f; else tC = (tProg - kTargetYellowStart) / (1.0f - kTargetYellowStart);
                }
                const juce::Colour col = baseGrey.interpolatedWith (yellow, tC).withAlpha (alpha);
                const float w  = juce::jmap (tC, 0.0f, 1.0f, 2.0f, 3.0f);
                g.setColour (col);
                g.drawLine (x0, y0, x1, y1, w);
                ++segs;
            }
            DBG("DrawSeries " << label << ": present=" << presentCount << " segs=" << segs
                << " yRaw[min,max]=" << yMin << "," << yMax << " alpha=" << alpha);
        };

        // Target first (lighter), then User (stronger)
        drawSeries("Target", yT_, presentT_, 0.55f, false);
        drawSeries("User",   yU_, presentU_, 1.00f, true);

        // --- Start markers (exact start times) ---
        auto drawStartIcon = [&] (const SeriesView& S)
        {
            if (!S.valid()) return;
            float amp = 0.0f, cen = (float) cmin; uint8_t pres = 0;
            sampleAtAbsolute (S, S.t0, amp, cen, pres);
            const float x = mapX (S.t0);
            const float y = mapY (amp);
            auto R = juce::Rectangle<float> (12.0f, 12.0f).withCentre ({ x, y });
            if (startIcon_.isValid()) {
                g.drawImageWithin (startIcon_, (int) R.getX(), (int) R.getY(), (int) R.getWidth(), (int) R.getHeight(), juce::RectanglePlacement::centred);
            } else {
                g.setColour(juce::Colours::orange);
                g.fillEllipse(R);
                g.setColour(juce::Colours::black.withAlpha(0.6f));
                g.drawEllipse(R, 1.2f);
            }
        };
        if (!overlayMode) {
            auto drawStartMarker = [&] (const SeriesView& S, bool isUser)
            {
                if (!S.valid()) return;
                float amp = 0.0f, cen = (float) cmin; uint8_t pres = 0;
                sampleAtAbsolute (S, S.t0, amp, cen, pres);
                const float x = mapX (S.t0);
                const float y = (isUser ? mapYUser(amp) : mapYTarget(amp));
                auto R = juce::Rectangle<float> (12.0f, 12.0f).withCentre ({ x, y });
                if (startIcon_.isValid()) {
                    g.drawImageWithin (startIcon_, (int) R.getX(), (int) R.getY(), (int) R.getWidth(), (int) R.getHeight(), juce::RectanglePlacement::centred);
                } else {
                    g.setColour(isUser ? juce::Colours::orange : juce::Colours::steelblue);
                    g.fillEllipse(R);
                    g.setColour(juce::Colours::black.withAlpha(0.6f));
                    g.drawEllipse(R, 1.2f);
                }
            };
            drawStartMarker(T,false);
            drawStartMarker(U,true);
        }

        drawAxes (g, plot, xAxis, tStart, span);

        DBG("Summary Nu=" << Nu << " Nt=" << Nt
            << " tU0=" << tU0 << " tU1=" << tU1
            << " tT0=" << tT0 << " tT1=" << tT1
            << " N=" << N);

        int cntU = std::accumulate(presentU_.begin(), presentU_.end(), 0);
        int cntT = std::accumulate(presentT_.begin(), presentT_.end(), 0);
    DBG("PresenceCounts user=" << cntU << " target=" << cntT);

    // Print first few amplitude samples for each (raw data) for deeper inspection
    auto dumpFirst = [] (const std::vector<float>& v, int k){ juce::String s; for (int i=0;i<juce::jmin(k,(int)v.size());++i){ s << v[(size_t)i] << (i<k-1?", ":""); } return s; };
    DBG("FirstUserAmps=" << dumpFirst(user_.amps, 8));
    DBG("FirstTargetAmps=" << dumpFirst(target_.amps, 8));
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

    void drawAxes (juce::Graphics& g,
                   juce::Rectangle<float> plot,
                   juce::Rectangle<float> xAxis,
                   double tStartSeconds,
                   double spanSeconds)
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
            g.drawText (secs (tStartSeconds + (double) f * spanSeconds),
                        juce::Rectangle<int> ((int) x - 28, (int) xAxis.getY(), 56, (int) xAxis.getHeight()),
                        juce::Justification::centredTop);
        }
    }
};
