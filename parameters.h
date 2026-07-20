#pragma once

#include <cstdlib>

constexpr size_t SAMPLE_RATE = 44100;
constexpr size_t NUMBER_OF_SINGERS = 20;
constexpr double TIME_ELAPSED = 1.0 / SAMPLE_RATE;
constexpr size_t WAVEFORM_LENGTH = 2205;
constexpr size_t SIMULATION_LENGTH = SAMPLE_RATE * 200;
constexpr float GLOBAL_LOWEST_FREQUENCY = 200.f;
constexpr float GLOBAL_HIGHEST_FREQUENCY = 784.f;
constexpr float SHORTEST_BREATH_LENGTH = 3.f;
constexpr float LONGEST_BREATH_LENGTH = 8.f;
constexpr float ATTACK_LENGTH = 0.5f;
constexpr float RELEASE_LENGTH = 0.5f;
