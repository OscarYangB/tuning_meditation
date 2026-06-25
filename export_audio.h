#pragma once

#include <iostream>
#include <fstream>

template<typename T>
inline void write_to_filestream(std::ofstream& stream, const T& data) {
	stream.write(reinterpret_cast<const char*>(&data), sizeof(data));
}

inline void normalize_audio(float* data, size_t size) {
	float highest_sample = 0.f;
	for (size_t i = 0; i < size; i++) {
		highest_sample = std::max(std::abs(data[i]), highest_sample);
	}
	for (size_t i = 0; i < size; i++) {
		data[i] /= highest_sample;
	}
}

inline void export_audio(float* data, size_t size) { // TODO
	normalize_audio(data, size);

	std::ofstream output_file("audio_export.wav", std::ios::binary);

	output_file << "RIFF";
	uint32_t file_size = size * sizeof(float) + 44 - 8;
	write_to_filestream(output_file, file_size);
	output_file << "WAVE";
	output_file << "fmt ";
	write_to_filestream(output_file, uint32_t{16});
	write_to_filestream(output_file, uint16_t{3});
	write_to_filestream(output_file, uint16_t{1});
	write_to_filestream(output_file, uint32_t{44100});
	write_to_filestream(output_file, uint32_t{44100 * sizeof(float)});
	write_to_filestream(output_file, uint16_t{sizeof(float)});
	write_to_filestream(output_file, uint16_t{sizeof(float) * 8});
	output_file << "data";
	write_to_filestream(output_file, size * sizeof(float));

	output_file.write(reinterpret_cast<const char*>(data), size * sizeof(float));
	output_file.close();
}
