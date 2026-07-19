#pragma once

#include <cstdlib>

inline float rand_float(float min, float max) { // TODO use normal distribution instead of uniform and better distribution for notes
	return (max - min) * (static_cast<double>(rand()) / RAND_MAX) + min;
}
