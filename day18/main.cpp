
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
	int64_t x;
	int64_t y;

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

	return tmpInstruction;
}

Instruction parseInstructionPart2(std::string_view sv) {
	Instruction tmpInstruction;

	sv.remove_prefix(2);

	auto lenStrEnd = sv.find_first_of(' ');

	sv.remove_prefix(lenStrEnd);
	sv.remove_prefix(3);

	auto distanceSv = sv.substr(0, 5);
	auto distanceStr = std::string{ distanceSv };
	tmpInstruction.amount = std::stoi(distanceStr, nullptr, 16);

	sv.remove_prefix(5);

	auto direction = sv[0];
	switch (direction) {
	case '0':
		tmpInstruction.directionDiff = Point{ 1, 0 };
		break;
	case '1':
		tmpInstruction.directionDiff = Point{ 0, 1 };
		break;
	case '2':
		tmpInstruction.directionDiff = Point{ -1, 0 };
		break;
	default:
	case '3':
		tmpInstruction.directionDiff = Point{ 0, -1 };
		break;
	}

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

// Scanline system
// assumption, the line does not self intersect
// track horizontal and vertical lines in separate multimaps
// have a map of vertical columns of change, these are the rows to the left, right and on any movement point
// vvvvv.vvvvv
// .#########.
// .#.......#.
// .###.....#.
// ...#.....#.
// ...#.....#.
// .###...###.
// .#.....#...
// .##....###.
// ..#......#.
// ..########.
// for any row of change, go through the line lists to see how many cells there are
// we assume that we start outside, and each line crossing, we transition inside, then outside.
// lines themselves are always inside
// if there is a gap in columns of change, we reuse the last calculated value, and multiply by the gap

uint64_t findPart2Area(const std::vector<Instruction>& instructions) {
	std::set<int64_t> columnsOfChange;
	columnsOfChange.insert(-1);
	columnsOfChange.insert(0);
	columnsOfChange.insert(1);

	std::map<int64_t, std::map<int64_t, int64_t>> rowSpans;
	//std::map<int64_t, std::set<int64_t>> columnSpans;

	Point currentPos{ 0, 0 };

	Point topLeftInclusive = currentPos;
	Point bottomRightInclusive = currentPos;

	for (auto& instruction : instructions) {
		Point destination = currentPos + Point{ instruction.directionDiff.x * instruction.amount, instruction.directionDiff.y * instruction.amount };

		columnsOfChange.insert(destination.x);
		columnsOfChange.insert(destination.x - 1);
		columnsOfChange.insert(destination.x + 1);

		topLeftInclusive.x = std::min(topLeftInclusive.x, destination.x);
		topLeftInclusive.y = std::min(topLeftInclusive.y, destination.y);
		bottomRightInclusive.x = std::max(bottomRightInclusive.x, destination.x);
		bottomRightInclusive.y = std::max(bottomRightInclusive.y, destination.y);

		if (instruction.directionDiff.x == 0) {
			auto minY = std::min(currentPos.y, destination.y);
			auto maxY = std::max(currentPos.y, destination.y);

			//columnSpans[currentPos.x].insert(minY);
			//columnSpans[currentPos.x].insert(maxY);
		} else {
			auto minX = std::min(currentPos.x, destination.x);
			auto maxX = std::max(currentPos.x, destination.x);

			rowSpans[currentPos.y].insert(std::make_pair(minX, maxX));
		}

		currentPos = destination;
	}

	topLeftInclusive.x-=2;
	topLeftInclusive.y-=2;

	bottomRightInclusive.x+=2;
	bottomRightInclusive.y+=2;

	uint64_t area = 0;

	uint64_t prevRowArea = 0;

	for (int64_t x = topLeftInclusive.x; x < bottomRightInclusive.x;) {
		auto nextColIt = columnsOfChange.upper_bound(x);
		if (nextColIt == columnsOfChange.end()) {
			x = bottomRightInclusive.x;
			continue;
		}

		auto nextX = *nextColIt;
		if (nextX > (x + 1)) {
			// we have a skip, add the area
			auto gapToMultiply = nextX - x;

			area += gapToMultiply * prevRowArea;
		}

		x = nextX;

		prevRowArea = 0;

		bool inside = false;

		//auto xColumnSetIt = columnSpans.find(x);

		for (int64_t y = topLeftInclusive.y; y < bottomRightInclusive.y;) {
			for (auto yS = y; yS < bottomRightInclusive.y;) {
				auto rowElem = rowSpans.upper_bound(yS);

				// std::optional<std::set<int64_t>::iterator> columnElem;
				// if (xColumnSetIt != columnSpans.end()) {
				// 	columnElem = xColumnSetIt->second.upper_bound(yS);
				// }

				// check if rowElem is a row of spans, it might not overlap with our current x, so check if it does.
				// If it doesn't, set yS to the value of rowElem

				if (rowElem == rowSpans.end()) {
					// we have reached the end, just exit out
					y = bottomRightInclusive.y;
					break;
				}

				auto rowElemColumnSpan = rowElem->second.upper_bound(x);
				if (rowElemColumnSpan != rowElem->second.begin()) {
					rowElemColumnSpan--;
				}

				// we now have a span, check if it overlaps
				if (rowElemColumnSpan->first <= x && rowElemColumnSpan->second >= x) {
					// we have a match
					// increase area if jump from y to yS and inside,
					// otherwise just jump
					auto increaseSpan = rowElem->first - y + 1;

					if (inside) {
						prevRowArea += increaseSpan;
					}
					inside = !inside;
					y = rowElem->first;
					break;
				} else {
					// it doesn't overlap, increase yS and loop again
					yS = rowElem->first;
				}
			}
		}

		area += prevRowArea;
	}

	return area;
}

uint64_t findPart2AreaV2(const std::vector<Instruction>& instructions) {
	// go from instructions to points
	Point currentPos{ 0, 0 };
	std::vector<Point> points;

	int64_t perimeter = 0;

	for (auto& ins : instructions) {
		auto newPos = currentPos + Point{ ins.directionDiff.x * ins.amount, ins.directionDiff.y * ins.amount };

		points.push_back(newPos);

		perimeter += (ins.amount);

		currentPos = newPos;
	}

	uint64_t area = 0;

	for (int i = 0; i < points.size(); i++) {
		auto next = (i + 1) % points.size();

		area += ((points[i].x * points[next].y) - (points[i].y * points[next].x));
	}

	area /= 2;

	return area + (perimeter / 2) + 1;

}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day18.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	auto nonEmptyLines = lines | std::views::filter([](auto sv) { return !sv.empty(); });
	auto instructions = nonEmptyLines | std::views::transform(parseInstruction) | std::ranges::to<std::vector>();

	auto part2Instructions = nonEmptyLines | std::views::transform(parseInstructionPart2) | std::ranges::to<std::vector>();

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

	auto part2Area = findPart2Area(part2Instructions);
	auto part2AreaV2 = findPart2AreaV2(part2Instructions);

	map.debugPrint();

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	fmt::print("Processed 1: {}\n", dugCells);
	fmt::print("Processed 2: {}\n", part2AreaV2);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
