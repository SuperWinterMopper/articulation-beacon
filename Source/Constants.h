#include <JuceHeader.h>

#pragma once

static const size_t NUM_EXERCISES = 4;

enum class ViewOptions {
	HOME,
	EX1,
	EX2,
	EX3,
	EX4
};

//Identifier for the viewing window state of the app
static juce::Identifier viewState("viewOption");

//For scoreState: For which line score is on (0,1,2...) or "Whole" for displaying whole score
static juce::Identifier scoreView("scoreView");

//For scoreState: Boolean, if video is actively in playback. Also correspond to metronome playback (so false -> both video and metronome are not playing)
static juce::Identifier isVideoPlaying("isVideoPlaying");

static juce::Identifier isAnalyzing("isAnalyzing");
static juce::Identifier tempo("tempo");