
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
	std::optional<uint32_t> color;
	bool dug;
	bool outside;
};

struct Map {
	Point topLeftInclusive;
	Point bottomRightInclusive;
	std::map<Point, Block> map;

	void debugPrint() {
		std::string printStr;

		for (int y = topLeftInclusive.y; y <= bottomRightInclusive.y; y++) {
			for (int x = topLeftInclusive.x; x <= bottomRightInclusive.x; x++) {
				if (map.contains(Point{ x, y }) && map[Point{x, y}].dug) {
					printStr += '#';
				} else {
					printStr += '.';
				}
			}

			printStr += '\n';
		}

		fmt::print("{}\n", printStr);
	}
};

struct Instruction {
	Point directionDiff;
	int amount;
	uint32_t color;
};

Instruction parseInstruction(std::string_view sv) {
	auto direction = sv[0];

	Instruction tmpInstruction;

	switch (direction) {
	case 'R':
		tmpInstruction.directionDiff = Point{ 1, 0 };
		break;
	case 'D':
		tmpInstruction.directionDiff = Point{ 0, 1 };
		break;
	case 'L':
		tmpInstruction.directionDiff = Point{ -1, 0 };
		break;
	default:
	case 'U':
		tmpInstruction.directionDiff = Point{ 0, -1 };
		break;
	}

	sv.remove_prefix(2);

	auto lenStrEnd = sv.find_first_of(' ');
	tmpInstruction.amount = std::stoi(std::string{ sv.begin(), sv.begin() + lenStrEnd });

	sv.remove_prefix(lenStrEnd);
	sv.remove_prefix(3);

	auto colorSv = sv.substr(0, 6);
	auto colorStr = std::string{ colorSv };
	tmpInstruction.color = std::stoi(colorStr, nullptr, 16);

	return tmpInstruction;
}

std::array<Point, 4> plusOffsets{ {
		{ -1, 0},
	{ 1, 0},
	{ 0, 1},
	{ 0, -1}
} };

void digOutHole(Map& map) {
	// expand the map by 1 in each direction
	map.topLeftInclusive.x--;
	map.topLeftInclusive.y--;
	map.bottomRightInclusive.x++;
	map.bottomRightInclusive.y++;

	std::queue<Point> pointsToCheckOutside;
	pointsToCheckOutside.push(map.topLeftInclusive);
	// flood the outside
	while (!pointsToCheckOutside.empty()) {
		auto point = pointsToCheckOutside.front();
		pointsToCheckOutside.pop();

		if (point.x < map.topLeftInclusive.x || point.y < map.topLeftInclusive.y || point.x > map.bottomRightInclusive.x || point.y > map.bottomRightInclusive.y) {
			continue;
		}

		bool expandFromPoint = false;

		if (!map.map.contains(point)) {
			map.map[point] = Block{ std::nullopt, false, true };
			expandFromPoint = true;
		} else if (map.map.at(point).outside == false && map.map.at(point).dug == false) {
			map.map[point].outside = true;
			expandFromPoint = true;
		}

		if (expandFromPoint) {
			for (int i = 0; i < 4; i++) {
				pointsToCheckOutside.push(point + plusOffsets[i]);
			}
		}
	}

	for (int y = map.topLeftInclusive.y; y < map.bottomRightInclusive.y; y++) {
		for (int x = map.topLeftInclusive.x; x < map.bottomRightInclusive.x; x++) {
			if (!map.map.contains(Point{x, y})) {
				map.map[Point{ x, y }] = Block{ std::nullopt, true, false };
			}
		}
	}
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day18.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	auto nonEmptyLines = lines | std::views::filter([](auto sv) { return !sv.empty(); });
	auto instructions = nonEmptyLines | std::views::transform(parseInstruction) | std::ranges::to<std::vector>();

	Map map;
	map.map[Point{ 0, 0 }] = Block{ std::nullopt, true, false };
	map.topLeftInclusive = Point{ 0, 0 };
	map.bottomRightInclusive = Point{ 0, 0 };

	Point currentPos{ 0, 0 };

	for (auto& instruction : instructions) {
		for (int i = 0; i < instruction.amount; i++) {
			currentPos = currentPos + instruction.directionDiff;
			map.map[currentPos] = Block{ std::nullopt, true, false };

			map.topLeftInclusive.x = std::min(map.topLeftInclusive.x, currentPos.x);
			map.topLeftInclusive.y = std::min(map.topLeftInclusive.y, currentPos.y);
			map.bottomRightInclusive.x = std::max(map.bottomRightInclusive.x, currentPos.x);
			map.bottomRightInclusive.y = std::max(map.bottomRightInclusive.y, currentPos.y);
		}
	}

	digOutHole(map);

	auto dugCells = std::ranges::count_if(map.map, [](auto& pair) {
		return pair.second.dug;
	});

	map.debugPrint();

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	fmt::print("Processed 1: {}\n", dugCells);
	//fmt::print("Processed 2: {}\n", maxPoints);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
