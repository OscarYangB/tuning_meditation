#include "singer.h"
#include "random.h"

constexpr size_t NUMBER_OF_HARMONICS = 10;

std::array<float, WAVEFORM_LENGTH> make_waveform() { // TODO different timbres and vowels
	std::array<float, WAVEFORM_LENGTH> result{};

	float amplitude = 1.f;
	for (size_t harmonic_index = 1; harmonic_index < NUMBER_OF_HARMONICS; harmonic_index++) {
		const float FREQUENCY = harmonic_index * (1.f / WAVEFORM_LENGTH); // TODO
		amplitude *= 0.5f;
		float random = rand_float(0.8f, 1.2f);
		for (size_t i = 0; i < WAVEFORM_LENGTH; i++) {
			result[i] += random * amplitude * std::sin(i * FREQUENCY * M_PI * 2.0);
		}
	}

	return result;
}


size_t Singer::get_waveform_index_from_progress() const {
	return std::floor(WAVEFORM_LENGTH * waveform_progress);
}

void Singer::change_note() {
	if (note_select_mode == NoteSelectMode::RANDOM) {
		// TODO consonance optimization and avoid existing note
		current_frequency = rand_float(lowest_frequency, highest_frequency);
		note_select_mode = NoteSelectMode::LISTEN;
	} else {
		// TODO polyphonic pitch detection
		note_select_mode = NoteSelectMode::RANDOM;
	}

	breath_length = rand_float(SHORTEST_BREATH_LENGTH, LONGEST_BREATH_LENGTH);
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

	breath_progress += TIME_ELAPSED;
	if (breath_progress > breath_length) {
		change_note();
		breath_progress -= breath_length;
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
