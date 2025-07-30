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

//Base Identifier for the viewing window state of the app
static juce::Identifier viewState("viewOption");

//=============================================================================
//Base Identifier for the score/DSP state of the exercise component
static juce::Identifier scoreStateIdentifier("scoreStateIdentifier");

//Below is all properties of scoreState:
	
//For scoreState: For which line score is on (0-indexed so 0,1,2...) or "Whole" for displaying whole score
static juce::Identifier scoreView("scoreView");

//For scoreState: Boolean, if video is actively in playback. DOES NOT correspond to metronome playback
static juce::Identifier isVideoPlaying("isVideoPlaying");

//For scoreState: Boolean, if video is muted
static juce::Identifier isVideoMuted("isVideoMuted");

//For scoreState: boolean, if the metronome is current playing 
static juce::Identifier isMetronomePlaying("isMetronomePlaying");

//For scoreState: Boolean, true => audio playback muted, user is playing and we are presenting graphical feedback. 
static juce::Identifier isAnalyzing("isAnalyzing");

//For scoreState: Is exactly 0 (slow or default tempo) or 1 (fast tempo) 
static juce::Identifier tempo("tempo");