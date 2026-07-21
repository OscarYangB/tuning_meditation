#pragma once

#include <random>

// TODO normal distribution

inline float rand_float(float min, float max) {
	return (max - min) * (static_cast<double>(rand()) / RAND_MAX) + min;
}

inline int rand_int(int min_inclusive, int max_exclusive) {
	return rand() % (max_exclusive - min_inclusive) + min_inclusive;
}
