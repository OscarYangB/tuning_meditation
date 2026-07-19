#pragma once

#include <vector>

constexpr size_t SAMPLE_RATE = 44100;
constexpr double TIME_ELAPSED = 1.0 / SAMPLE_RATE;
constexpr size_t WAVEFORM_LENGTH = 2205;
constexpr size_t SIMULATION_LENGTH = SAMPLE_RATE * 100;

constexpr float GLOBAL_LOWEST_FREQUENCY = 200.f;
constexpr float GLOBAL_HIGHEST_FREQUENCY = 1567.98f; // Highest note for a "soprano" singer is G6

constexpr float SHORTEST_BREATH_LENGTH = 3.f;
constexpr float LONGEST_BREATH_LENGTH = 8.f;

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
