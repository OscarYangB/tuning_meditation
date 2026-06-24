#include "waveform.h"
#include "export_audio.h"

constexpr size_t NUMBER_OF_SECONDS_TO_RUN = 10;

struct PositionedSinger {
	Singer* singer;
	float x;
	float y;
};

void randomly_position_singers(std::vector<Singer>& singers) {
	std::vector<PositionedSinger> positioned_singers{};

	for (Singer& singer : singers) {
		const float x = static_cast<float>(rand()) / RAND_MAX;
		const float y = static_cast<float>(rand()) / RAND_MAX;
		positioned_singers.emplace_back(PositionedSinger{&singer, x, y});
	}

	for (int i = 0; i < singers.size(); i++) {
		for (int j = 0; j < singers.size(); j++) {
			if (i == j) {
				continue;
			}
			float distance = std::sqrt(std::pow(positioned_singers[i].x - positioned_singers[j].x, 2) +
									   std::pow(positioned_singers[i].y - positioned_singers[j].y, 2));
			positioned_singers[i].singer->add_connection(positioned_singers[j].singer, 1.f / distance);
			positioned_singers[j].singer->add_connection(positioned_singers[i].singer, 1.f / distance);
		}
	}
}

int main() {
	std::vector<Singer> singers{};
	for (int i = 0; i < 3; i++) {
		singers.emplace_back(make_waveform(), GLOBAL_LOWEST_FREQUENCY, GLOBAL_HIGHEST_FREQUENCY);
	}

	randomly_position_singers(singers);

	std::cout << "Initialized" << "\n";

	for (int i = 0; i < SAMPLE_RATE * NUMBER_OF_SECONDS_TO_RUN; i++) {
		for (Singer& singer : singers) {
			singer.process();
		}
		for (Singer& singer : singers) {
			singer.send();
		}
	}

	std::cout << "Done" << "\n";

	std::array<float, MEMORY_SIZE> result = singers[0].get_result();
	export_audio(result.data(), MEMORY_SIZE);
}
