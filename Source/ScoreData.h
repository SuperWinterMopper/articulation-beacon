#pragma once

//this is really only for testing 
static const int defaultBPM = 90;

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