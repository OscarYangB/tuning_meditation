#pragma once

#include <cstdlib>

constexpr size_t SAMPLE_RATE = 44100;
constexpr size_t NUMBER_OF_SINGERS = 20;
constexpr double TIME_ELAPSED = 1.0 / SAMPLE_RATE;
constexpr size_t SIMULATION_LENGTH = SAMPLE_RATE * 200;

constexpr size_t WAVEFORM_LENGTH = 2205;
constexpr float WAVEFORM_PARTIAL_AMPLITUDE_VARIANCE = 0.2f;
constexpr float WAVEFORM_PARTIAL_FALLOFF = 0.7f;
constexpr float WAVEFORM_PARTIAL_FALLOFF_VARIANCE = 0.1f;
constexpr size_t WAVEFORM_HARMONICS = 10;

constexpr float GLOBAL_LOWEST_FREQUENCY = 200.f;
constexpr float GLOBAL_HIGHEST_FREQUENCY = 800.f;
constexpr float SHORTEST_BREATH_LENGTH = 3.f;
constexpr float LONGEST_BREATH_LENGTH = 11.f;
constexpr float ATTACK_LENGTH = 0.5f;
constexpr float RELEASE_LENGTH = 0.5f;
constexpr float MAX_DISTANCE = 0.5f;
constexpr float PITCH_VARIANCE = 0.05f;
constexpr size_t PITCH_CANDIDATES = 20;
