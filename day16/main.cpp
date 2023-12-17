
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

struct Beam {
	Point currentPos;
	Point directionChange;

	std::strong_ordering operator<=>(const Beam&) const = default;
	bool operator==(const Beam&) const = default;
};

int countEnergizedBeams(Block& map, Beam startBeam) {
	std::set<Point> activatedPoints;
	std::queue<Beam> beamsToProcess;
	std::set<Beam> processedBeams;

	beamsToProcess.push(startBeam);

	while (!beamsToProcess.empty()) {

		auto beam = beamsToProcess.front();
		beamsToProcess.pop();

		if (processedBeams.contains(beam)) {
			continue;
		}
		processedBeams.insert(beam);

		// advance the beam
		auto newPos = beam.currentPos + beam.directionChange;

		if (!map.map.contains(newPos)) {
			continue;
		}

		activatedPoints.insert(newPos);

		if (map.map.at(newPos) == '.') {
			beamsToProcess.push(Beam{ newPos, beam.directionChange });
		} else if (map.map.at(newPos) == '|' && beam.directionChange.x == 0) {
			beamsToProcess.push(Beam{ newPos, beam.directionChange });
		} else if (map.map.at(newPos) == '|') {
			beamsToProcess.push(Beam{ newPos, {0, 1} });
			beamsToProcess.push(Beam{ newPos, {0, -1} });
		} else if (map.map.at(newPos) == '-' && beam.directionChange.y == 0) {
			beamsToProcess.push(Beam{ newPos, beam.directionChange });
		} else if (map.map.at(newPos) == '-') {
			beamsToProcess.push(Beam{ newPos, {1, 0} });
			beamsToProcess.push(Beam{ newPos, {-1, 0} });
		} else if (map.map.at(newPos) == '\\' && beam.directionChange.x == 1) {
			beamsToProcess.push(Beam{ newPos, {0, 1} });
		} else if (map.map.at(newPos) == '\\' && beam.directionChange.x == -1) {
			beamsToProcess.push(Beam{ newPos, {0, -1} });
		} else if (map.map.at(newPos) == '\\' && beam.directionChange.y == 1) {
			beamsToProcess.push(Beam{ newPos, {1, 0} });
		} else if (map.map.at(newPos) == '\\' && beam.directionChange.y == -1) {
			beamsToProcess.push(Beam{ newPos, {-1, 0} });
		} else if (map.map.at(newPos) == '/' && beam.directionChange.x == 1) {
			beamsToProcess.push(Beam{ newPos, {0, -1} });
		} else if (map.map.at(newPos) == '/' && beam.directionChange.x == -1) {
			beamsToProcess.push(Beam{ newPos, {0, 1} });
		} else if (map.map.at(newPos) == '/' && beam.directionChange.y == 1) {
			beamsToProcess.push(Beam{ newPos, {-1, 0} });
		} else if (map.map.at(newPos) == '/' && beam.directionChange.y == -1) {
			beamsToProcess.push(Beam{ newPos, {1, 0} });
		}
	}

	return activatedPoints.size();
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day16.txt");
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
				tmpBlock.map[Point{ i, row }] = values[i];
			}
			tmpBlock.width = values.size();
			tmpBlock.height = row + 1;
		}

		return tmpBlock;
	}) | std::ranges::to<std::vector>();

	auto map = maps[0];

	auto topLeftCount = countEnergizedBeams(map, Beam{ { -1, 0 }, { 1, 0 } });

	int maxPoints = 0;

	for (int i = 0; i < map.height; i++) {
		Beam leftBeam{ {-1, i}, {1, 0} };
		maxPoints = std::max(maxPoints, countEnergizedBeams(map, leftBeam));

		Beam rightBeam{ {map.width, i}, {-1, 0} };
		maxPoints = std::max(maxPoints, countEnergizedBeams(map, rightBeam));
	}
	for (int i = 0; i < map.width; i++) {
		Beam topBeam{ {i, -1}, {0, 1} };
		maxPoints = std::max(maxPoints, countEnergizedBeams(map, topBeam));

		Beam bottomBeam{ {i, map.height}, {0, -1} };
		maxPoints = std::max(maxPoints, countEnergizedBeams(map, bottomBeam));
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	//debugPrintMap(galaxies);

	fmt::print("Processed 1: {}\n", topLeftCount);
	fmt::print("Processed 2: {}\n", maxPoints);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
