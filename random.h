#pragma once

#include <cstdlib>

inline float rand_float(float min, float max) { // TODO use normal distribution instead of uniform
	return (max - min) * (static_cast<double>(rand()) / RAND_MAX) + min;
}
