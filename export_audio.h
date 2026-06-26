#pragma once

#include <iostream>
#include <fstream>

template<typename T>
inline void write(std::ofstream& stream, const T& data) {
	stream.write(reinterpret_cast<const char*>(&data), sizeof(data));
}

inline void normalize_audio(float* data, size_t length) {
	float highest_sample = 0.f;
	for (size_t i = 0; i < length; i++) {
		highest_sample = std::max(std::abs(data[i]), highest_sample);
	}
	for (size_t i = 0; i < length; i++) {
		data[i] /= highest_sample;
	}
}

inline void export_audio(float* data, size_t length) {
	const size_t SAMPLE_SIZE = sizeof(float);
	const size_t CHANNELS = 1;
	const size_t SAMPLE_RATE = 44100;

	normalize_audio(data, length);

	std::ofstream output_file("audio_export.wav", std::ios::binary);

	output_file << "RIFF";
	uint32_t file_size = length * SAMPLE_SIZE + 44 - 8;
	write(output_file, file_size);
	output_file << "WAVE";
	output_file << "fmt ";
	write(output_file, uint32_t{16});
	write(output_file, uint16_t{3});
	write(output_file, uint16_t{CHANNELS});
	write(output_file, uint32_t{SAMPLE_RATE});
	write(output_file, uint32_t{SAMPLE_RATE * SAMPLE_SIZE});
	write(output_file, uint16_t{SAMPLE_SIZE});
	write(output_file, uint16_t{SAMPLE_SIZE * 8});
	output_file << "data";
	write(output_file, length * SAMPLE_SIZE);
	output_file.write(reinterpret_cast<const char*>(data), length * SAMPLE_SIZE);

	output_file.close();
}

inline void export_audio_stereo(float* left, float* right, size_t length) {
	const size_t SAMPLE_SIZE = sizeof(float);
	const size_t CHANNELS = 2;
	const size_t SAMPLE_RATE = 44100;

	normalize_audio(left, length);
	normalize_audio(right, length);

	std::ofstream output_file("audio_export.wav", std::ios::binary);

	output_file << "RIFF";
	uint32_t file_size = length * CHANNELS * SAMPLE_SIZE + 44 - 8;
	write(output_file, file_size);
	output_file << "WAVE";
	output_file << "fmt ";
	write(output_file, uint32_t{16});
	write(output_file, uint16_t{3});
	write(output_file, uint16_t{CHANNELS});
	write(output_file, uint32_t{SAMPLE_RATE});
	write(output_file, uint32_t{SAMPLE_RATE * SAMPLE_SIZE * CHANNELS});
	write(output_file, uint16_t{SAMPLE_SIZE * CHANNELS});
	write(output_file, uint16_t{SAMPLE_SIZE * 8});
	output_file << "data";
	write(output_file, length * sizeof(float) * CHANNELS);
	for (int i = 0; i < length; i++) {
		output_file.write(reinterpret_cast<const char*>(&left[i]), SAMPLE_SIZE);
		output_file.write(reinterpret_cast<const char*>(&right[i]), SAMPLE_SIZE);
	}

	output_file.close();
}
