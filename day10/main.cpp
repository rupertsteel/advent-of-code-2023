
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

#include <fmt/ranges.h>
#include <fmt/chrono.h>

using namespace std::string_view_literals;

struct Point {
	int x;
	int y;

	std::strong_ordering operator<=>(const Point&) const = default;
	bool operator==(const Point&) const = default;


	Point operator+(const Point& other) const {
		return Point{ x + other.x, y + other.y };
	}
};

std::array<Point, 4> plusOffsets{ {
		{ -1, 0},
	{ 1, 0},
	{ 0, 1},
	{ 0, -1}
} };

std::array<std::set<char>, 4> validConnectors{ {
	{ '-', 'L', 'F'},
	{ '-', 'J', '7'},
	{'|', 'L', 'J'},
	{'|', '7',  'F'}
} };

struct ProcessPoint {
	Point point;
	int distanceFromStart;

	std::strong_ordering operator<=>(const ProcessPoint& other) const {
		const auto distanceDiff = distanceFromStart <=> other.distanceFromStart;

		if (distanceDiff != std::strong_ordering::equal) {
			return distanceDiff;
		}

		return point <=> other.point;
	}
};



int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day10.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	auto nonEmptyLines = lines | std::views::filter([](auto sv) { return !sv.empty(); });

	std::optional<Point> startPoint;

	std::map<Point, char> map;
	int y = 0;
	for (const auto& line : nonEmptyLines) {

		for (int x = 0; x < line.size(); x++) {
			map.insert(std::make_pair(Point{ x, y }, line[x]));

			if (line[x] == 'S') {
				startPoint = Point{ x,y };
			}
		}

		++y;
	}

	if (!startPoint) {
		throw std::runtime_error("Invalid map");
	}

	std::map<Point, int> distanceMap;

	distanceMap[*startPoint] = 0;

	std::priority_queue<ProcessPoint, std::vector<ProcessPoint>, std::greater<>> pointsToProcess;

	for (int i = 0; i < 4; i++) {
		auto& offset = plusOffsets[i];

		auto checkLocation = *startPoint + offset;

		if (!map.contains(checkLocation)) {
			continue;
		}

		auto ch = map.at(checkLocation);

		if (validConnectors[i].contains(ch)) {
			pointsToProcess.push(ProcessPoint{ checkLocation, 1 });
		}
	}

	int maxDistance = 0;

	std::map<char, std::vector<Point>> nextOffsetsForTile {
		{'|', { {0, -1}, {0, 1}}},
		{'-', { {1, 0}, {-1, 0}} },
		{'J', { {-1, 0}, {0, -1}}},
		{'7', {{-1, 0}, {0, 1}}},
		{'F', {{1, 0}, {0, 1}}},
		{'L', {{1, 0}, {0,-1}}},
	};

	while (!pointsToProcess.empty()) {
		auto point = pointsToProcess.top();
		pointsToProcess.pop();

		if (distanceMap.contains(point.point)) {
			continue;
		}

		distanceMap[point.point] = point.distanceFromStart;
		maxDistance = std::max(maxDistance, point.distanceFromStart);

		auto ch = map.at(point.point);

		const auto& nextOffsets = nextOffsetsForTile.at(ch);

		for (auto& offset : nextOffsets) {
			auto loc = point.point + offset;

			pointsToProcess.push(ProcessPoint{ loc, point.distanceFromStart + 1 });
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;


	fmt::print("Processed 1: {}\n", maxDistance);
	//fmt::print("Processed 2: {}\n", frontPredictSum);


	fmt::print("Took {}\n", dur);

	return 0;
}
