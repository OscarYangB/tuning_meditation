#pragma once

#include <complex>

// radix-2 decimation-in-time FFT
// Assumes length is a power of 2
inline void fast_fourier_transform(const float* input, size_t length, std::complex<float>* output, size_t s = 1) {
	if (length == 1) {
		output[0] = input[0];
		return;
	}

	fast_fourier_transform(input, length / 2, output, s * 2);
	fast_fourier_transform(input + s, length / 2, output + length / 2, s * 2);
	for (int k = 0; k < length / 2; k++) {
		auto p = output[k];
		auto q = std::exp(std::complex<float>(0.f, -2.0 * M_PI * k / length)) * output[k+length/2];
		output[k] = p + q;
		output[k + length / 2] = p - q;
	}
}

inline std::vector<float> fast_fourier_transform(const float* input, size_t length, size_t padded_length) {
	std::vector<float> padded_input{};
	std::copy(input, input + length, std::back_inserter(padded_input));
	padded_input.resize(padded_length);

	std::vector<std::complex<float>> output{};
	output.resize(padded_length);
	fast_fourier_transform(padded_input.data(), padded_length, output.data());

	std::vector<float> result{};
	result.reserve(padded_length);
	for (std::complex<float> complex_value : output) {
		result.emplace_back(std::abs(complex_value));
	}
	return result;
}

inline void hann_window(float* data, size_t length) {
	for (size_t i = 0; i < length; i++) {
		data[i] *= std::pow(std::sin(M_PI * i / length), 2.f);
	}
}

inline float index_to_frequency(size_t index, size_t fft_size, size_t sample_rate) {
	if (index == 0) {
		return 0;
	}
	float wavelength_in_samples = static_cast<float>(fft_size) / index;
	float wavelength_in_seconds = wavelength_in_samples / sample_rate;
	return 1.f / wavelength_in_seconds;
}

inline std::vector<float> identify_peaks(const std::vector<float>& frequencies, size_t fft_size, size_t sample_rate) {
	struct CandidatePeak {
		size_t index;
		float salience = 0.f;
	};

	std::vector<CandidatePeak> candidates{};

	float max_frequency = 0.f;
	for (float frequency : frequencies) {
		max_frequency = std::max(max_frequency, frequency);
	}

	static constexpr float THRESHOLD_COEFFICIENT = 0.2f;
	const float THRESHOLD = THRESHOLD_COEFFICIENT * max_frequency;
	for (size_t i = 2; i < std::floor(frequencies.size() / 3.f) - 2; i++) {
		if (frequencies[i] < THRESHOLD) {
			continue;
		}

		bool is_local_max = frequencies[i-2] < frequencies[i-1] && frequencies[i-1] < frequencies[i] && frequencies[i] > frequencies[i+1] && frequencies[i+1] > frequencies[i+2];
		if (!is_local_max) {
			continue;
		}
		candidates.push_back(CandidatePeak{i});
	}

	for (CandidatePeak& candidate : candidates) {
		for (int p = 1; p <= 3; p++) {
			candidate.salience += std::pow(frequencies[candidate.index*p], 0.25f);
		}
	}
	float max_salience = 0.f;
	for (CandidatePeak& candidate : candidates) {
		max_salience = std::max(max_salience, candidate.salience);
	}
	std::vector<float> results{};
	const float SALIENCE_THRESHOLD = std::pow(0.4f, 0.25f) * max_salience;
	for (CandidatePeak& candidate : candidates) {
		if (candidate.salience > SALIENCE_THRESHOLD) {
			results.emplace_back(index_to_frequency(candidate.index, fft_size, sample_rate));
		}
	}
	return results;
}

inline float calculate_dissonance_of_partial(float frequency_1, float frequency_2) {
	float critical_bandwidth = 1.72f * std::pow((frequency_2 + frequency_1) / 2.f, 0.65f);
	float x = std::abs(frequency_2 - frequency_1) / critical_bandwidth;
	if (x > 1.2f) {
		return -1.f;
	}
	const float a = 0.25f;
	return std::pow((std::exp(1.f)*x)/a * std::exp(-x/a), 2.f); // Bigand, Parncutt & Lerdahl 1996
}

// William Hutchinson & Leon Knopoff, The acoustic component of western consonance, 1978
inline float calculate_dissonance(const std::vector<float>& frequencies_1, const std::vector<float>& frequencies_2, size_t fft_size, size_t sample_rate) {
	float dissonance = 0.f;
	float normalizing_scale = 0.f;

	for (size_t i = 0; i < frequencies_1.size(); i++) {
		float frequency_1 = index_to_frequency(i, fft_size, sample_rate);
		if (frequency_1 < 20.f) {
			continue;
		}
		normalizing_scale += frequencies_1[i] * frequencies_1[i];

		for (size_t j = i + 1; j < frequencies_2.size(); j++) {
			float frequency_2 = index_to_frequency(j, fft_size, sample_rate);
			float g = calculate_dissonance_of_partial(frequency_1, frequency_2);
			if (g < 0.f) {
				break;
			}
			dissonance += frequencies_1[i] * frequencies_2[j] * g;
			dissonance += frequencies_1[j] * frequencies_2[i] * g;
		}
	}

	return (0.5f * dissonance) / normalizing_scale;
}
