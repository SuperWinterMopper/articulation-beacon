#pragma once

//this is really only for testing 
static const int defaultBPM = 90;

struct ExerciseDataStruct {
	std::string name;
	float onset_thresh;
	int bpm;
	int max_num_notes_per_beat;
	float min_time_between;
	float sustain_thresh_coeff;
	float sustain_thresh;
	std::vector<int> lineBeatLengths;

	ExerciseDataStruct(std::string name, float onset_thresh, int bpm, int max_num_notes_per_beat, float sustain_thresh_coeff, std::vector<int> lineBeatLengths) :
		name(name),
		onset_thresh(onset_thresh),
		bpm(bpm),
		max_num_notes_per_beat(max_num_notes_per_beat),
		sustain_thresh_coeff(sustain_thresh_coeff),
		min_time_between(60.0 / (bpm * max_num_notes_per_beat)),
		sustain_thresh(sustain_thresh_coeff * onset_thresh),
		lineBeatLengths(lineBeatLengths)
	{}
};

static const std::vector<ExerciseDataStruct> ExerciseData = {
    {
        "ex1WholeMod.mp4",
        0.0803f,     //onset_thresh
        90,          //bpm
        2,           //max_num_notes_per_beat
        0.7333f,     //sustain_thresh_coeff
        {32, 32, 32} //lineBeatLengths
    },
    {
        "ex1WholeModF.mp4",
        0.1555f,     //onset_thresh
        120,         //bpm
        2,           //max_num_notes_per_beat
        0.4500f,     //sustain_thresh_coeff
        {32, 32, 32} //lineBeatLengths
    },
    {
        "ex2WholeMod.mp4",
        0.1142f,     //onset_thresh
        80,          //bpm
        1,           //max_num_notes_per_beat
        0.4167f,     //sustain_thresh_coeff
        {16, 16, 16, 8} //lineBeatLengths
    },
    {
        "ex3WholeMod.mp4",
        0.0800f,     //onset_thresh
        100,         //bpm
        2,           //max_num_notes_per_beat
        1.5833f,     //sustain_thresh_coeff
        {12, 12, 16} //lineBeatLengths
    },
    {
        "ex3WholeModF.mp4",
        0.0800f,     //onset_thresh
        130,         //bpm
        2,           //max_num_notes_per_beat
        1.3500f,     //sustain_thresh_coeff
        {12, 12, 16} //lineBeatLengths
    },
    {
        "ex4WholeMod.mp4",
        0.1870f,     //onset_thresh
        90,          //bpm
        4,           //max_num_notes_per_beat
        0.4500f,     //sustain_thresh_coeff
        {20, 24, 20} //lineBeatLengths
    },
    {
        "ex4WholeModF.mp4",
        0.0800f,     //onset_thresh
        120,         //bpm
        2,           //max_num_notes_per_beat
        1.0833f,     //sustain_thresh_coeff
        {20, 24, 20} //lineBeatLengths
    },
    {
        "ex5WholeMod.mp4",
        0.1021f,     //onset_thresh
        64,          //bpm
        2,           //max_num_notes_per_beat
        0.4667f,     //sustain_thresh_coeff
        {26, 26, 26, 13} //lineBeatLengths
    },
    {
        "ex5WholeModF.mp4",
        0.1045f,     //onset_thresh
        86,          //bpm
        2,           //max_num_notes_per_beat
        0.6500f,     //sustain_thresh_coeff
        {26, 26, 26, 13} //lineBeatLengths
    },
    {
        "ex6WholeMod.mp4",
        0.1021f,     //onset_thresh
        70,          //bpm
        4,           //max_num_notes_per_beat
        0.9000f,     //sustain_thresh_coeff
        {12, 12, 8}  //lineBeatLengths
    },
    {
        "ex6WholeModF.mp4",
        0.1142f,     //onset_thresh
        90,          //bpm
        4,           //max_num_notes_per_beat
        1.1500f,     //sustain_thresh_coeff
        {12, 12, 8}  //lineBeatLengths
    },
    {
        "ex7WholeMod.mp4",
        0.1215f,     //onset_thresh
        70,          //bpm
        2,           //max_num_notes_per_beat
        0.3000f,     //sustain_thresh_coeff
        {0}          //lineBeatLengths (weird pickup case)
    },
    {
        "ex8WholeMod.mp4",
        0.1167f,     //onset_thresh
        55,          //bpm
        4,           //max_num_notes_per_beat
        0.6000f,     //sustain_thresh_coeff
        {8, 12, 12}  //lineBeatLengths
    },
    {
        "ex8WholeModF.mp4",
        0.1191f,     //onset_thresh
        80,          //bpm
        4,           //max_num_notes_per_beat
        0.9333f,     //sustain_thresh_coeff
        {8, 12, 12}  //lineBeatLengths
    },
    {
        "ex9WholeMod.mp4",
        0.0800f,     //onset_thresh
        100,         //bpm
        4,           //max_num_notes_per_beat
        1.6500f,     //sustain_thresh_coeff
        {12, 12, 8}  //lineBeatLengths
    },
    {
        "ex10WholeMod.mp4",
        0.1433f,     //onset_thresh
        120,         //bpm
        3,           //max_num_notes_per_beat
        0.9167f,     //sustain_thresh_coeff
        {12, 18, 18} //lineBeatLengths
    }
};

static const std::vector<std::vector<int>> exerciseTempo = {
	{80, 120},
	{80},
	{100, 130},
	{90, 120},
	{64, 86},
	{70, 90},
	{70},
	{55, 80},
	{100},
	{120}
};

static const std::vector<std::vector<int>> lineBeatLength = {
	{32, 32, 32},
	{16, 16, 16, 8},
	{12, 12, 16},
	{20, 24, 20},
	{26, 26, 26, 13},
	{12, 12, 8},
	{0}, // this one is weird with a pick up, let's not deal with it for now
	{8, 12, 12},
	{12, 12, 8},
	{12, 18, 18}
};