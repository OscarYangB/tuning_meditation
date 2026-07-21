#include "singer.h"
#include "random.h"
#include "frequency.h"

std::array<float, WAVEFORM_LENGTH> make_waveform() {
	std::array<float, WAVEFORM_LENGTH> result{};

	float amplitude = 1.f;
	for (size_t harmonic_index = 1; harmonic_index < WAVEFORM_HARMONICS; harmonic_index++) {
		const float FREQUENCY = harmonic_index * (1.f / WAVEFORM_LENGTH);
		amplitude *= (WAVEFORM_PARTIAL_FALLOFF + rand_float(-WAVEFORM_PARTIAL_FALLOFF_VARIANCE, WAVEFORM_PARTIAL_FALLOFF_VARIANCE));
		float random = rand_float(1.f - WAVEFORM_PARTIAL_AMPLITUDE_VARIANCE, 1.f + WAVEFORM_PARTIAL_AMPLITUDE_VARIANCE);
		for (size_t i = 0; i < WAVEFORM_LENGTH; i++) {
			result[i] += random * amplitude * std::sin(i * FREQUENCY * M_PI * 2.0);
		}
	}

	return result;
}

float Singer::Waveform::get() {
	float sample = data[std::floor(WAVEFORM_LENGTH * progress)];
	progress += frequency * frequency_modulation * TIME_ELAPSED;
	if (progress > 1.0) {
		progress -= 1.0;
	}
	return sample;
}

void Singer::change_note() {
	breath_length = rand_float(SHORTEST_BREATH_LENGTH, LONGEST_BREATH_LENGTH);
	volume = rand_float(0.f, 1.f);

	vibrato_waveform.frequency = rand_float(MIN_VIBRATO_FREQUENCY, MAX_VIBRATO_FREQUENCY);
	vibrato_amplitude = rand_float(0.f, 1.f);

	if (std::sqrt(short_rms_sum / SHORT_RMS_LENGTH) < std::sqrt(long_rms_sum / LONG_RMS_LENGTH) * STOPPING_THRESHOLD) {
		is_stopped = true;
	}

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
		std::array<Candidate, PITCH_CANDIDATES> candidates{};
		for (int i = 0; i < PITCH_CANDIDATES; i++) {
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
		if (frequency_peaks.empty()) {
			is_stopped = true;
		} else {
			waveform.frequency = frequency_peaks[rand_int(0, frequency_peaks.size())];
			waveform.frequency *= rand_float(1.f - PITCH_VARIANCE, 1.f + PITCH_VARIANCE);
		}
		note_select_mode = NoteSelectMode::RANDOM;
	}
}

Singer::Singer(size_t id, std::array<float, WAVEFORM_LENGTH> waveform, float lowest_frequency, float highest_frequency) {
	this->id = id;
	this->waveform = {waveform, 0.f, rand_float(0.f, 1.f)};
	this->lowest_frequency = lowest_frequency;
	this->highest_frequency = highest_frequency;
	memory.reserve(MAX_SIMULATION_LENGTH);
	std::array<float, WAVEFORM_LENGTH> vibrato_signal{};
	for (int i = 0; i < WAVEFORM_LENGTH; i++) {
		vibrato_signal[i] = std::sin(2.f * M_PI * (static_cast<float>(i) / WAVEFORM_LENGTH));
	}
	vibrato_waveform = {vibrato_signal, rand_float(MIN_VIBRATO_FREQUENCY, MAX_VIBRATO_FREQUENCY), rand_float(0.f, 1.f)};
	vibrato_amplitude = rand_float(0.f, 1.f);
	change_note();
}

void Singer::process() {
	if (!memory.empty()) {
		if (memory.size() > SHORT_RMS_LENGTH) {
			short_rms_sum -= std::pow(memory[memory.size() - 1 - SHORT_RMS_LENGTH], 2.f);
		}
		short_rms_sum += std::pow(memory[memory.size() - 1], 2.f);
		short_rms_sum = std::max(short_rms_sum, 0.f);

		if (memory.size() > LONG_RMS_LENGTH) {
			long_rms_sum -= std::pow(memory[memory.size() - 1 - LONG_RMS_LENGTH], 2.f);
		}
		long_rms_sum += std::pow(memory[memory.size() - 1], 2.f);
		long_rms_sum = std::max(long_rms_sum, 0.f);
	}

	memory.emplace_back();
}

void Singer::send() {
	if (is_stopped) {
		return;
	}

	waveform.frequency_modulation = 1.f + vibrato_waveform.get() * MAX_VIBRATO_AMPLITUDE * vibrato_amplitude;
	const float SAMPLE = waveform.get() * get_volume_envelope() * volume;

	for (Connection& connection : connections) {
		connection.singer->receive(SAMPLE);
	}

	breath_progress += TIME_ELAPSED;
	if (breath_progress > breath_length) {
		breath_progress -= breath_length;
		change_note();
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

float Singer::get_volume_envelope() {
	if (breath_progress > breath_length - RELEASE_LENGTH) {
		return 1.f - (breath_progress - breath_length + RELEASE_LENGTH) / RELEASE_LENGTH;
	} else if (breath_progress < ATTACK_LENGTH) {
		return breath_progress / ATTACK_LENGTH;
	}
	return 1.f;
}
