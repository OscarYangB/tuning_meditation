#pragma once

#include <vector>
#include "parameters.h"
#include "frequency.h"

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
		float frequency_modulation = 1.f;

		float get();
	};

	size_t id;
	Waveform waveform;
	float lowest_frequency;
	float highest_frequency;
	std::vector<float> memory;
	double breath_length;
	float volume;
	Waveform vibrato_waveform;
	float vibrato_amplitude;
	Convolution<CONVOLUTION_LENGTH> convolution{nullptr};

	std::vector<Connection> connections{};
	double breath_progress = 0.0;
	NoteSelectMode note_select_mode = NoteSelectMode::RANDOM;
	bool is_stopped = false;
	float long_rms_sum = 0.f;
	float short_rms_sum = 0.f;

	void change_note();
	float get_volume_envelope();

public:
	Singer(size_t id, std::array<float, WAVEFORM_LENGTH> waveform, float lowest_frequency, float highest_frequency, std::complex<float>* convolution_frequencies);
	void process();
	void send();
	void receive(float sample);
	void add_connection(Singer* singer, float weight);
	std::vector<float>& get_result();
	inline bool get_is_stopped() { return is_stopped; }
};

struct Connection {
	Singer* singer;
	float weight; // [0, 1]
};

std::array<float, WAVEFORM_LENGTH> make_waveform();
