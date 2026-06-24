#include "waveform.h"

constexpr size_t NUMBER_OF_HARMONICS = 10;

std::array<float, WAVEFORM_LENGTH> make_waveform() {
	std::array<float, WAVEFORM_LENGTH> result{};

	float amplitude = 1.f;
	for (size_t harmonic_index = 1; harmonic_index < NUMBER_OF_HARMONICS; harmonic_index++) {
		const float FREQUENCY = harmonic_index * (1.f / WAVEFORM_LENGTH); // TODO
		amplitude *= 0.5f;
		for (size_t i = 0; i < WAVEFORM_LENGTH; i++) {
			result[i] += amplitude * std::sin(i * FREQUENCY * M_PI * 2.0);
		}
	}

	return result;
}
