#include "singer.h"
#include "export_audio.h"
#include "random.h"

constexpr size_t NUMBER_OF_SINGERS = 10;

struct PositionedSinger {
	Singer* singer;
	float x;
	float y;
};

void randomly_position_singers(std::vector<Singer>& singers) { // TODO disconnect far-apart nodes
	std::vector<PositionedSinger> positioned_singers{};

	for (Singer& singer : singers) {
		const float x = rand_float(0.f, 1.f);
		const float y = rand_float(0.f, 1.f);
		positioned_singers.emplace_back(PositionedSinger{&singer, x, y});
	}

	for (int i = 0; i < singers.size(); i++) {
		for (int j = i + 1; j < singers.size(); j++) {
			float distance = (std::sqrt(std::pow(positioned_singers[i].x - positioned_singers[j].x, 2) +
										std::pow(positioned_singers[i].y - positioned_singers[j].y, 2))) / sqrt(2.0);
			positioned_singers[i].singer->add_connection(positioned_singers[j].singer, 1.f - distance);
			positioned_singers[j].singer->add_connection(positioned_singers[i].singer, 1.f - distance);
		}
	}
}

int main() {
	std::vector<Singer> singers{};
	for (size_t i = 0; i < NUMBER_OF_SINGERS; i++) {
		singers.emplace_back(i, make_waveform(), GLOBAL_LOWEST_FREQUENCY, GLOBAL_HIGHEST_FREQUENCY);
	}

	randomly_position_singers(singers);

	std::cout << "Initialized" << "\n";

	for (int i = 0; i < SIMULATION_LENGTH; i++) { // TOOD End simulation using excitable media
		for (Singer& singer : singers) {
			singer.process();
		}
		for (Singer& singer : singers) {
			singer.send();
		}
	}

	std::cout << "Done" << "\n";

	std::vector<float>& left = singers[0].get_result();
	std::vector<float>& right = singers[1].get_result();
	export_audio_stereo(left.data(), right.data(), SIMULATION_LENGTH);
}
