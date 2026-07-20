#pragma once

#include <vector>
#include "parameters.h"

enum class NoteSelectMode {
	RANDOM,
	LISTEN
};

struct Connection;

class Singer {
private:
	struct Waveform {
		std::array<float, WAVEFORM_LENGTH> data;
		float frequency;
		double progress = 0.0;

		float get();
	};

	size_t id;
	Waveform waveform;
	float lowest_frequency;
	float highest_frequency;
	std::vector<float> memory;
	double breath_length;

	std::vector<Connection> connections{};
	double breath_progress = 0.0;
	NoteSelectMode note_select_mode = NoteSelectMode::RANDOM;

	void change_note();

public:
	Singer(size_t id, std::array<float, WAVEFORM_LENGTH> waveform, float lowest_frequency, float highest_frequency);
	void process();
	void send();
	void receive(float sample);
	void add_connection(Singer* singer, float weight);
	std::vector<float>& get_result();
};

struct Connection {
	Singer* singer;
	float weight; // [0, 1]
};

std::array<float, WAVEFORM_LENGTH> make_waveform();
