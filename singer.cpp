#include "singer.h"
#include "random.h"
#include "frequency.h"

constexpr size_t NUMBER_OF_HARMONICS = 10;

std::array<float, WAVEFORM_LENGTH> make_waveform() { // TODO different timbres and vowels
	std::array<float, WAVEFORM_LENGTH> result{};

	float amplitude = 1.f;
	for (size_t harmonic_index = 1; harmonic_index < NUMBER_OF_HARMONICS; harmonic_index++) {
		const float FREQUENCY = harmonic_index * (1.f / WAVEFORM_LENGTH);
		amplitude *= (0.5f + rand_float(-0.05f, 0.05f));
		float random = rand_float(0.5f, 1.5f);
		for (size_t i = 0; i < WAVEFORM_LENGTH; i++) {
			result[i] += random * amplitude * std::sin(i * FREQUENCY * M_PI * 2.0);
		}
	}

	return result;
}

float Singer::Waveform::get() {
	float sample = data[std::floor(WAVEFORM_LENGTH * progress)];
	progress += frequency * TIME_ELAPSED;
	if (progress > 1.0) {
		progress -= 1.0;
	}
	return sample;
}

void Singer::change_note() {
	breath_length = rand_float(SHORTEST_BREATH_LENGTH, LONGEST_BREATH_LENGTH);

	static constexpr size_t ANALYSIS_SAMPLE_LENGTH = 4096;
	static constexpr size_t PADDED_LENGTH = 16384;
	if (memory.size() < ANALYSIS_SAMPLE_LENGTH) {
		waveform.frequency = rand_float(lowest_frequency, highest_frequency);
		return;
	}
	std::array<float, ANALYSIS_SAMPLE_LENGTH> samples_to_analyze{};
	std::copy(memory.end() - ANALYSIS_SAMPLE_LENGTH, memory.end(), samples_to_analyze.begin());
	hann_window(samples_to_analyze.data(), samples_to_analyze.size());
	std::vector<float> frequencies = fast_fourier_transform(samples_to_analyze.data(), samples_to_analyze.size(), PADDED_LENGTH);
	frequencies.resize(frequencies.size() / 2);

	if (note_select_mode == NoteSelectMode::RANDOM) {
		struct Candidate {
			float frequency;
			float dissonance;
		};
		static constexpr size_t NUMBER_OF_CANDIDATES = 20;
		std::array<Candidate, NUMBER_OF_CANDIDATES> candidates{};
		for (int i = 0; i < NUMBER_OF_CANDIDATES; i++) {
			candidates[i] = {rand_float(lowest_frequency, highest_frequency)};
			Waveform candidate_waveform = {waveform.data, candidates[i].frequency};
			std::array<float, PADDED_LENGTH> candidate_signal{};
			for (int j = 0; j < PADDED_LENGTH; j++) {
				candidate_signal[j] = candidate_waveform.get();
			}
			hann_window(candidate_signal.data(), PADDED_LENGTH);
			std::vector<float> candidate_frequencies = fast_fourier_transform(candidate_signal.data(), PADDED_LENGTH, PADDED_LENGTH);
			candidate_frequencies.resize(frequencies.size() / 2);
			candidates[i].dissonance = calculate_dissonance(frequencies, candidate_frequencies, PADDED_LENGTH, SAMPLE_RATE);
		}
		const Candidate* best_candidate = &candidates[0];
		for (int i = 1; i < candidates.size(); i++) {
			if (candidates[i].dissonance < best_candidate->dissonance) {
				best_candidate = &candidates[i];
			}
		}
		waveform.frequency = best_candidate->frequency;
		note_select_mode = NoteSelectMode::LISTEN;
	} else {
		std::vector<float> frequency_peaks = identify_peaks(frequencies, PADDED_LENGTH, SAMPLE_RATE);
		auto iter = std::remove_if(frequency_peaks.begin(), frequency_peaks.end(), [this](float frequency){
			return frequency < lowest_frequency || frequency > highest_frequency;
		});
		frequency_peaks.erase(iter, frequency_peaks.end());
		waveform.frequency = frequency_peaks[std::floor(rand_float(0, frequency_peaks.size()))];
		//waveform.frequency = rand_float(waveform.frequency * 0.95f, waveform.frequency * 1.05f);
		note_select_mode = NoteSelectMode::RANDOM;
	}
}

Singer::Singer(size_t id, std::array<float, WAVEFORM_LENGTH> waveform, float lowest_frequency, float highest_frequency) {
	this->id = id;
	this->waveform = {waveform};
	this->lowest_frequency = lowest_frequency;
	this->highest_frequency = highest_frequency;
	memory.reserve(SIMULATION_LENGTH);
	change_note();
}

void Singer::process() {
	memory.emplace_back();
}

void Singer::send() {
	const float SAMPLE = waveform.get();

	for (Connection& connection : connections) {
		connection.singer->receive(SAMPLE);
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
