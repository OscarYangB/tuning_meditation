#include "parameters.h"
#include "singer.h"
#include "export_audio.h"
#include "random.h"

struct PositionedSinger {
	Singer* singer;
	float x;
	float y;
};

void randomly_position_singers(std::vector<Singer>& singers) {
	std::vector<PositionedSinger> positioned_singers{};

	for (Singer& singer : singers) {
		const float x = rand_float(0.f, 1.f);
		const float y = rand_float(0.f, 1.f);
		positioned_singers.emplace_back(PositionedSinger{&singer, x, y});
	}

	for (int i = 0; i < singers.size(); i++) {
		for (int j = i + 1; j < singers.size(); j++) {
			float distance = std::sqrt(std::pow(positioned_singers[i].x - positioned_singers[j].x, 2) +
									   std::pow(positioned_singers[i].y - positioned_singers[j].y, 2));
			if (distance > MAX_DISTANCE) {
				continue; // Don't add connections to nodes that are really far away
			}
			positioned_singers[i].singer->add_connection(positioned_singers[j].singer, 1.f / distance); // Sound in a gas decreases inverse-proportionally
			positioned_singers[j].singer->add_connection(positioned_singers[i].singer, 1.f / distance);
		}
	}
}

int main() {
	srand(1293812);

	std::vector<std::complex<float>> impulse_frequencies{};
	if constexpr (USE_CONVOLUTION) {
		impulse_frequencies = std::vector<std::complex<float>>(CONVOLUTION_LENGTH);
		std::vector<float> impulse = import_audio("impulse_2.raw");
		impulse.resize(CONVOLUTION_LENGTH);
		fast_fourier_transform(impulse.data(), impulse.size(), impulse_frequencies.data());
	}

	std::vector<Singer> singers{};
	for (size_t i = 0; i < NUMBER_OF_SINGERS; i++) {
		singers.emplace_back(i, make_waveform(), GLOBAL_LOWEST_FREQUENCY, GLOBAL_HIGHEST_FREQUENCY, impulse_frequencies.data());
	}

	randomly_position_singers(singers);

	std::cout << "Initialized" << "\n";

	for (int i = 0; i < MAX_SIMULATION_LENGTH; i++) {
		for (Singer& singer : singers) {
			singer.process();
		}
		for (Singer& singer : singers) {
			singer.send();
		}

		bool all_stopped = true;
		for (Singer& singer : singers) {
			all_stopped &= singer.get_is_stopped();
		}
		if (all_stopped) {
			break;
		}
	}

	if constexpr (USE_CONVOLUTION) {
		for (int i = 0; i < CONVOLUTION_LENGTH; i++) {
			for (Singer& singer : singers) {
				singer.process();
			}
			for (Singer& singer : singers) {
				singer.send();
			}
		}
	}

	std::cout << "Done" << '\n';

	for (int i = 0; i < singers.size(); i++) {
		export_audio(singers[i].get_result().data(), singers[i].get_result().size(), "output_" + std::to_string(i) + ".wav");
	}
}
