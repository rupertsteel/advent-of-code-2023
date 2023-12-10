
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
	int maxX = 0;

	int y = 0;
	for (const auto& line : nonEmptyLines) {

		for (int x = 0; x < line.size(); x++) {
			map.insert(std::make_pair(Point{ x, y }, line[x]));

			if (line[x] == 'S') {
				startPoint = Point{ x,y };
			}

			maxX = std::max(maxX, x);
		}

		++y;
	}

	if (!startPoint) {
		throw std::runtime_error("Invalid map");
	}

	std::map<Point, int> distanceMap;

	std::map<Point, char> doubleGrid;

	for (const auto& elem : map) {
		auto newLoc = Point{ 1 + 2 * elem.first.x, 1 + 2 * elem.first.y };

		doubleGrid[newLoc] = elem.second;
	}

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

			// add blocker to double grid
			auto startDoubleLoc = Point{ 1 + 2 * startPoint->x, 1 + 2 * startPoint->y };
			auto blockerDoubleLoc = startDoubleLoc + offset;

			doubleGrid[blockerDoubleLoc] = 'B';
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

			// add blocker to double grid
			auto startDoubleLoc = Point{ 1 + 2 * point.point.x, 1 + 2 * point.point.y };
			auto blockerDoubleLoc = startDoubleLoc + offset;

			doubleGrid[blockerDoubleLoc] = 'B';
		}
	}

	// replace points not connected to the grid with '.'
	for (const auto& loc : map) {
		auto ch = loc.second;

		if (ch == '|' || ch == '-' || ch == 'J' || ch == 'F' || ch == 'L' || ch == '7') {
			if (!distanceMap.contains(loc.first)) {
				auto doubleGridLoc = Point{ 1 + 2 * loc.first.x, 1 + 2 * loc.first.y };

				doubleGrid[doubleGridLoc] = '.';
			}
		}
	}

	// floodFill double grid, when the floodfill encounters an empty location (within bounds), we fill with '#',
	// when we encounter '.', replace with 'O'
	// bounds are (-2, -2), (maxX + 2, y + 2)

	std::queue<Point> floodQueue;
	floodQueue.push(Point{ -2, -2 });

	int doubleGridMaxX = (maxX + 2) * 2 + 1;
	int doubleGridMaxY = (y + 2) * 2 + 1;

	std::set<Point> donePoints;

	while (!floodQueue.empty()) {
		auto nextPoint = floodQueue.front();
		floodQueue.pop();

		if (donePoints.contains(nextPoint)) {
			continue;
		}

		if (nextPoint.x < -2 || nextPoint.y < -2 || nextPoint.x > doubleGridMaxX || nextPoint.y > doubleGridMaxY) {
			continue;
		}

		if (doubleGrid.contains(nextPoint)) {
			auto val = doubleGrid.at(nextPoint);

			if (val == 'B' || val == '|' || val == '-' || val == 'J' || val == 'F' || val == '7' || val == 'L' || val == 'S') {
				continue;
			}

			if (val == '.') {
				doubleGrid[nextPoint] = 'O';
			}
		} else {
			doubleGrid[nextPoint] = '#';
		}

		for (auto& offset : plusOffsets) {
			auto newPoint = nextPoint + offset;

			floodQueue.push(newPoint);
		}

		donePoints.insert(nextPoint);
	}

	auto countNotTouched = std::ranges::count_if(doubleGrid, [](auto& val) {
		return val.second == '.';
		});

	std::string mapPrintTest;
	for (int y = -2; y <= doubleGridMaxY; y++) {
		for (int x = -2; x <= doubleGridMaxX; x++) {
			if (doubleGrid.contains(Point{x, y})) {
				mapPrintTest.push_back(doubleGrid.at(Point{ x, y }));
			} else {
				mapPrintTest.push_back(' ');
			}
		}

		mapPrintTest.push_back('\n');
	}

	fmt::print("{}\n", mapPrintTest);

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;


	fmt::print("Processed 1: {}\n", maxDistance);
	fmt::print("Processed 2: {}\n", countNotTouched);


	fmt::print("Took {}\n", dur);

	return 0;
}
