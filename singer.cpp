#include "singer.h"

size_t Singer::get_waveform_index_from_progress() const {
	return std::floor(WAVEFORM_LENGTH * waveform_progress);
}

void Singer::change_note() {
	current_frequency = ((highest_frequency - lowest_frequency) * static_cast<float>(rand()) / RAND_MAX) + lowest_frequency;
}

Singer::Singer(std::array<float, WAVEFORM_LENGTH> waveform, float lowest_frequency, float highest_frequency) {
	this->waveform = waveform;
	this->lowest_frequency = lowest_frequency;
	this->highest_frequency = highest_frequency;
	memory.reserve(SIMULATION_LENGTH);
	change_note();
}

void Singer::process() {
	memory.emplace_back();
}

void Singer::send() {
	const float SAMPLE = waveform[get_waveform_index_from_progress()];

	for (Connection& connection : connections) {
		connection.singer->receive(SAMPLE);
	}

	waveform_progress += current_frequency * TIME_ELAPSED;
	if (waveform_progress > 1.0) {
		waveform_progress -= 1.0;
	}
}

void Singer::receive(float sample) {
	memory[memory.size() - 1] += sample;
}

void Singer::add_connection(Singer* singer, float weight) {
	connections.emplace_back(Connection{singer, weight});
}

std::vector<float>& Singer::get_result() {
	return memory;
}
