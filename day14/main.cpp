
#include <fmt/format.h>
#include <iostream>
#include <fstream>
#include <ranges>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <map>
#include <queue>
#include <valarray>
#include <regex>
#include <set>
#include <ratio>
#include <execution>
#include <unordered_map>

#include <fmt/ranges.h>
#include <fmt/chrono.h>

using namespace std::string_view_literals;
using namespace std::string_literals;

struct Point {
	int x;
	int y;

	std::strong_ordering operator<=>(const Point&) const = default;
	bool operator==(const Point&) const = default;


	Point operator+(const Point& other) const {
		return Point{ x + other.x, y + other.y };
	}
};

struct Block {
	int width;
	int height;
	std::map<Point, char> map;

	void slideUp() {
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (map.contains(Point{x, y}) && map[Point{x, y}] == 'O') {
					int newY = y;
					while (newY > 0) {
						if (map.contains(Point{ x, newY -1 })) {
							break;
						}
						newY--;
					}

					map.erase(Point{ x, y });
					map[Point{ x, newY }] = 'O';
				}
			}
		}
	}

	void slideLeft() {
		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				if (map.contains(Point{ x, y }) && map[Point{ x, y }] == 'O') {
					int newX = x;
					while (newX > 0) {
						if (map.contains(Point{ newX - 1, y })) {
							break;
						}
						newX--;
					}

					map.erase(Point{ x, y });
					map[Point{ newX, y}] = 'O';
				}
			}
		}
	}

	void slideDown() {
		for (int y = height - 1; y >= 0; y--) {
			for (int x = 0; x < width; x++) {
				if (map.contains(Point{ x, y }) && map[Point{ x, y }] == 'O') {
					int newY = y;
					while (newY < (height - 1)) {
						if (map.contains(Point{ x, newY + 1 })) {
							break;
						}
						newY++;
					}

					map.erase(Point{ x, y });
					map[Point{ x, newY }] = 'O';
				}
			}
		}
	}

	void slideRight() {
		for (int x = width - 1; x >= 0; x--) {
			for (int y = 0; y < height; y++) {
				if (map.contains(Point{ x, y }) && map[Point{ x, y }] == 'O') {
					int newX = x;
					while (newX < (width - 1)) {
						if (map.contains(Point{ newX + 1, y })) {
							break;
						}
						newX++;
					}

					map.erase(Point{ x, y });
					map[Point{ newX, y}] = 'O';
				}
			}
		}
	}

	void slideCycle() {
		slideUp();
		slideLeft();
		slideDown();
		slideRight();
	}

	uint64_t scoreMap() {
		uint64_t totalScore = 0;

		for (auto val : map) {
			if (val.second == 'O') {
				totalScore += height - val.first.y;
			}
		}

		return totalScore;
	}

	void debugPrint() {
		std::string printStr;

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (map.contains(Point{ x, y })) {
					printStr += map[Point{ x, y }];
				} else {
					printStr += '.';
				}
			}

			printStr += '\n';
		}

		fmt::print("{}\n", printStr);
	}
};

struct CycleTracking {
	uint64_t load;
	uint64_t cycle;
};


int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day14.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	//auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	//auto nonEmptyLines = lines | std::views::filter([](auto sv) { return !sv.empty(); });

	auto blocks = std::string_view{ input } | std::views::split("\n\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	
	auto maps = blocks | std::views::transform([](std::string_view sv) {
		Block tmpBlock;

		auto rowStr = std::views::zip(std::views::iota(0), sv | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); })
			| std::views::filter([](auto sv) { return !sv.empty(); }));

		for (auto val : rowStr) {
			auto row = std::get<0>(val);

			auto values = std::get<1>(val);

			for (int i = 0; i < values.size(); i++) {
				if (values[i] != '.') {
					tmpBlock.map[Point{ i, row }] = values[i];
				}
			}
			tmpBlock.width = values.size();
			tmpBlock.height = row + 1;
		}

		return tmpBlock;
	}) | std::ranges::to<std::vector>();

	//maps[0].slideUp();
	auto mapCopy = maps[0];
	mapCopy.slideUp();
	uint64_t weight = mapCopy.scoreMap();

	uint64_t billionCycleWeight = 0;

	uint64_t cycle = 0;

	std::map<std::map<Point, char>, std::deque<CycleTracking>> cycleTracking;

	while (true) {
		maps[0].slideCycle();
		cycle++;
		auto score = maps[0].scoreMap();

		cycleTracking[maps[0].map].push_back(CycleTracking{ score, cycle });
		if (cycleTracking[maps[0].map].size() > 10) {
			cycleTracking[maps[0].map].pop_front();
		}
		if (cycleTracking[maps[0].map].size() > 5) {
			// find the cycle length
			auto& vec = cycleTracking[maps[0].map];

			std::vector<uint64_t> cyclesDiffs;
			for (int i = 1; i < vec.size(); i++) {
				cyclesDiffs.push_back(vec[i].cycle - vec[i - 1].cycle);
			}

			if (std::all_of(cyclesDiffs.begin(), cyclesDiffs.end(), [&](auto elem) {
				return elem == cyclesDiffs[0];
			})) {
				// check if vec[0].cycle + cyclesDiffs[0] * ? == 1'000'000'000
				// 1'000'000'000 - vec[0].cycle = cyclesDiffs[0] * ?
				// (1'000'000'000 - vec[0].cycle) % cyclesDiffs[0] == 0

				if ((1'000'000'000ull - vec[0].cycle) % cyclesDiffs[0] == 0) {
					billionCycleWeight = vec[0].load;
					break;
				}
			}
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	//debugPrintMap(galaxies);

	fmt::print("Processed 1: {}\n", weight);
	fmt::print("Processed 2: {}\n", billionCycleWeight);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
