#pragma once

#include <vector>

constexpr size_t SAMPLE_RATE = 44100;
constexpr double TIME_ELAPSED = 1.0 / SAMPLE_RATE;
constexpr size_t WAVEFORM_LENGTH = 2205;
constexpr size_t SIMULATION_LENGTH = SAMPLE_RATE * 100;

constexpr float GLOBAL_LOWEST_FREQUENCY = 98.f; // Lowest note for a "bass" singer is G2
constexpr float GLOBAL_HIGHEST_FREQUENCY = 1567.98f; // Highest note for a "soprano" singer is G6

struct Connection;

class Singer {
private:
	std::array<float, WAVEFORM_LENGTH> waveform;
	float lowest_frequency;
	float highest_frequency;

	float current_frequency = 0.f;
	std::vector<Connection> connections{};
	std::vector<float> memory;
	double waveform_progress = 0.0;

	size_t get_waveform_index_from_progress() const;
	void change_note();

public:
	Singer(std::array<float, WAVEFORM_LENGTH> waveform, float lowest_frequency, float highest_frequency);
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
