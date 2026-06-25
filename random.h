#pragma once

#include <cstdlib>

inline float rand_float(float min, float max) {
	return (max - min) * static_cast<float>(rand()) / RAND_MAX + min;
}
