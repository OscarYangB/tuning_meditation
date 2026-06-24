#include "singer.h"

size_t Singer::get_waveform_index_from_progress() {
	return std::floor(WAVEFORM_LENGTH / waveform_progress);
}

Singer::Singer(std::array<float, WAVEFORM_LENGTH> waveform, float lowest_frequency, float highest_frequency) {
	this->waveform = waveform;
	this->lowest_frequency = lowest_frequency;
	this->highest_frequency = highest_frequency;
}

void Singer::process() {
	memory_index++;
	memory_index %= MEMORY_SIZE;
	memory[memory_index] = 0;
}

void Singer::send() {
	const size_t SAMPLE = waveform[get_waveform_index_from_progress()];

	for (Connection& connection : connections) {
		connection.singer->receive(SAMPLE);
	}

	waveform_progress += current_frequency * TIME_ELAPSED;
	if (waveform_progress > 1.0) {
		waveform_progress -= 1.0;
	}
}

void Singer::receive(float sample) {
	memory[memory_index] += sample;
}

void Singer::add_connection(Singer* singer, float weight) {
	connections.emplace_back(Connection{singer, weight});
}

std::array<float, MEMORY_SIZE> Singer::get_result() {
	return memory;
}
