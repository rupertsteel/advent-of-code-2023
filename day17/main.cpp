
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
	std::map<Point, int> map;

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
	int spacesLeft;

	std::strong_ordering operator<=>(const Beam&) const = default;
	bool operator==(const Beam&) const = default;
};

struct Path {
	Beam beam;
	int currentCost;
	int estMinFutureCost;

	std::strong_ordering operator<=>(const Path& other) const {
		return (currentCost + estMinFutureCost) <=> (other.currentCost + other.estMinFutureCost);
	}

	std::vector<Point> tracePoints;
};

// thinking about scores, we have a lower bound (assuming 1 cost to the end, and a upper bound assuming 9 cost to the end)
// Our heuristic assumes a 1 cost manhattan distance to the end

int manhattanDistance(Point a, Point b) {
	return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}
int weirdDistance(Point a, Point b) {
	int distX = std::abs(a.x - b.x);
	int distY = std::abs(a.y - b.y);

	int minDistYFromDistX = distX / 3;
	int minDistXFromDistY = distY / 3;

	int turnAroundDist = 4;

	return std::max(distX, minDistXFromDistY) + std::max(distY, minDistYFromDistX) + turnAroundDist;
}

int aStarSearch(const Block& map, Point startingPos, Point endPos) {

	std::map<Beam, int> exploredScores;

	std::priority_queue<Path, std::vector<Path>, std::greater<>> queue;

	queue.push(Path{ {startingPos, {1, 0}, 3}, 0, manhattanDistance(startingPos, endPos) });
	queue.push(Path{ {startingPos, {0, 1}, 3}, 0, manhattanDistance(startingPos, endPos) });

	bool foundEnd = false;

	int bestScore = std::numeric_limits<int>::max();

	uint64_t processed = 0;

	auto mapTotalExplorePoints = map.width * map.height * 4 * 3;
	auto collapseThreshold = mapTotalExplorePoints * 2;

	while (!queue.empty() && (!foundEnd || (queue.top().currentCost + queue.top().estMinFutureCost - 10) < bestScore)) {
		if (queue.size() > collapseThreshold) {
			auto sizeBefore = queue.size();
			std::map<Beam, Path> collapsedPaths;

			while (!queue.empty()) {
				auto val = queue.top();
				queue.pop();

				if (!collapsedPaths.contains(val.beam)) {
					collapsedPaths[val.beam] = val;
				} else {
					if (collapsedPaths[val.beam].currentCost > val.currentCost) {
						collapsedPaths[val.beam] = val;
					}
				}
			}

			for (auto& [key, value] : collapsedPaths) {
				queue.push(value);
			}

			auto sizeAfter = queue.size();

			fmt::println("Collapse, before: {}, after: {}, diff: {}", sizeBefore, sizeAfter, sizeBefore - sizeAfter);

			continue;
		}

		auto nextPath = queue.top();
		queue.pop();

		processed++;

		if ((processed & 0xFFFFFF) == 0) {
			auto mapExploreFraction = static_cast<float>(exploredScores.size()) / mapTotalExplorePoints;
			fmt::println("{} iterations, map is {}% explored, {} items in the queue, score {}/{}", processed, mapExploreFraction * 100.0f, queue.size(), nextPath.currentCost, nextPath.currentCost + nextPath.estMinFutureCost);
		}

		if (exploredScores.contains(nextPath.beam) && exploredScores.at(nextPath.beam) < (nextPath.currentCost + nextPath.estMinFutureCost)) {
			continue;
		}

		for (int i = nextPath.beam.spacesLeft; i > 0; i--) {
			auto workingBeam = nextPath.beam;
			workingBeam.spacesLeft = i;
			exploredScores[workingBeam] = (nextPath.currentCost + nextPath.estMinFutureCost);
		}

		auto newPos = nextPath.beam.currentPos + nextPath.beam.directionChange;
		if (newPos.x < 0 || newPos.y < 0 || newPos.x >= map.width || newPos.y >= map.height) {
			// skip out of bounds
			continue;
		}

		// add the movement cost
		int newCurrentCost = nextPath.currentCost + map.map.at(newPos);
		int newEstMinFutureCost = manhattanDistance(newPos, endPos);

		if (newPos == endPos) {
			foundEnd = true;
			bestScore = std::min(bestScore, newCurrentCost);
			continue;
		}

		// if we can move forward, do so
		if (nextPath.beam.spacesLeft > 1) {
			queue.push(Path{ {newPos, nextPath.beam.directionChange, nextPath.beam.spacesLeft - 1}, newCurrentCost, newEstMinFutureCost });
		}

		auto checkNextPathValid = [&](const Path& p) {
			auto nextPos = p.beam.currentPos + p.beam.directionChange;
			if (nextPos.x < 0 || nextPos.y < 0 || nextPos.x >= map.width || nextPos.y >= map.height) {
				return false;
			}

			if (exploredScores.contains(p.beam) && exploredScores.at(p.beam) < (p.currentCost + p.estMinFutureCost)) {
				return false;
			}

			return true;
		};

		// add in our turns
		if (nextPath.beam.directionChange.x == 0) {
			{
				Path tmpPath{ {newPos, {1, 0}, 3}, newCurrentCost, newEstMinFutureCost };
				if (checkNextPathValid(tmpPath)) {
					queue.push(tmpPath);
				}
			}
			{
				Path tmpPath{ {newPos, {-1, 0}, 3}, newCurrentCost, newEstMinFutureCost };
				if (checkNextPathValid(tmpPath)) {
					queue.push(tmpPath);
				}
			}
		} else {
			{
				Path tmpPath{ {newPos, {0, 1}, 3}, newCurrentCost, newEstMinFutureCost };
				if (checkNextPathValid(tmpPath)) {
					queue.push(tmpPath);
				}
			}
			{
				Path tmpPath{ {newPos, {0, -1}, 3}, newCurrentCost, newEstMinFutureCost };
				if (checkNextPathValid(tmpPath)) {
					queue.push(tmpPath);
				}
			}
		}
	}

	return bestScore;
}

void debugPrintMap(const Block& map, Path& path) {
	std::string tmpStr;

	for (int y = 0; y < map.height; y++) {
		for (int x = 0; x < map.width; x++) {
			bool printedPathVal = false;

			for (auto& point : path.tracePoints) {
				if (point.x == x && point.y == y) {
					tmpStr.push_back('*');
					printedPathVal = true;
				}
			}

			if (!printedPathVal) {
				tmpStr.push_back(map.map.at(Point{ x, y }) + '0');
			}
		}

		tmpStr.push_back('\n');
	}

	fmt::print("{}\n", tmpStr);
}

int aStarSearchV2(const Block& map, Point startingPos, Point endPos) {

	std::map<Beam, int> exploredScores;

	std::priority_queue<Path, std::vector<Path>, std::greater<>> queue;

	queue.push(Path{ {startingPos, {1, 0}, 10}, 0, manhattanDistance(startingPos, endPos) });
	queue.push(Path{ {startingPos, {0, 1}, 10}, 0, manhattanDistance(startingPos, endPos) });

	bool foundEnd = false;

	int bestScore = std::numeric_limits<int>::max();
	std::optional<Path> bestPath;

	uint64_t processed = 0;

	auto mapTotalExplorePoints = map.width * map.height * 4 * 10;
	auto collapseThreshold = mapTotalExplorePoints * 2;

	while (!queue.empty() && (!foundEnd || (queue.top().currentCost + queue.top().estMinFutureCost - 10) < bestScore)) {
		if (queue.size() > collapseThreshold) {
			auto sizeBefore = queue.size();
			std::map<Beam, Path> collapsedPaths;

			while (!queue.empty()) {
				auto val = queue.top();
				queue.pop();

				if (!collapsedPaths.contains(val.beam)) {
					collapsedPaths[val.beam] = val;
				} else {
					if (collapsedPaths[val.beam].currentCost > val.currentCost) {
						collapsedPaths[val.beam] = val;
					}
				}
			}

			for (auto& [key, value] : collapsedPaths) {
				queue.push(value);
			}

			auto sizeAfter = queue.size();

			fmt::println("Collapse, before: {}, after: {}, diff: {}", sizeBefore, sizeAfter, sizeBefore - sizeAfter);

			continue;
		}

		auto nextPath = queue.top();
		queue.pop();

		processed++;

		if ((processed & 0xFFFFFF) == 0) {
			auto mapExploreFraction = static_cast<float>(exploredScores.size()) / mapTotalExplorePoints;
			fmt::println("{} iterations, map is {}% explored, {} items in the queue, score {}/{}", processed, mapExploreFraction * 100.0f, queue.size(), nextPath.currentCost, nextPath.currentCost + nextPath.estMinFutureCost);
		}

		if (exploredScores.contains(nextPath.beam) && exploredScores.at(nextPath.beam) < (nextPath.currentCost + nextPath.estMinFutureCost)) {
			continue;
		}

		exploredScores[nextPath.beam] = (nextPath.currentCost + nextPath.estMinFutureCost);

		auto newPos = nextPath.beam.currentPos + nextPath.beam.directionChange;
		if (newPos.x < 0 || newPos.y < 0 || newPos.x >= map.width || newPos.y >= map.height) {
			// skip out of bounds
			continue;
		}

		// add the movement cost
		int newCurrentCost = nextPath.currentCost + map.map.at(newPos);
		int newEstMinFutureCost = manhattanDistance(newPos, endPos);

		if (newPos == endPos && nextPath.beam.spacesLeft <= 7) {
			foundEnd = true;
			if (newCurrentCost < bestScore) {
				bestScore = newCurrentCost;
				bestPath = nextPath;
			}
			continue;
		}

		// if we can move forward, do so
		if (nextPath.beam.spacesLeft > 1) {
			auto newPath = nextPath;
			newPath.beam = Beam{ newPos, nextPath.beam.directionChange, nextPath.beam.spacesLeft - 1 };
			newPath.currentCost = newCurrentCost;
			newPath.estMinFutureCost = newEstMinFutureCost;
			newPath.tracePoints.push_back(newPos);
			queue.push(newPath);
		}

		auto checkNextPathValid = [&](const Path& p) {
			auto nextPos = p.beam.currentPos + p.beam.directionChange;
			if (nextPos.x < 0 || nextPos.y < 0 || nextPos.x >= map.width || nextPos.y >= map.height) {
				return false;
			}

			if (exploredScores.contains(p.beam) && exploredScores.at(p.beam) < (p.currentCost + p.estMinFutureCost)) {
				return false;
			}

			return true;
		};

		Path newPath = nextPath;
		newPath.currentCost = newCurrentCost;
		newPath.estMinFutureCost = newEstMinFutureCost;
		newPath.tracePoints.push_back(newPos);

		// add in our turns if we can
		if (nextPath.beam.spacesLeft <= 7) {
			if (nextPath.beam.directionChange.x == 0) {
				{
					newPath.beam = Beam{ newPos, {1, 0}, 10 };
					if (checkNextPathValid(newPath)) {
						queue.push(newPath);
					}
				}
				{
					newPath.beam = Beam{ newPos, {-1, 0}, 10 };
					if (checkNextPathValid(newPath)) {
						queue.push(newPath);
					}
				}
			} else {
				{
					newPath.beam = Beam{ newPos, {0, 1}, 10 };
					if (checkNextPathValid(newPath)) {
						queue.push(newPath);
					}
				}
				{
					newPath.beam = Beam{ newPos, {0, -1}, 10 };
					if (checkNextPathValid(newPath)) {
						queue.push(newPath);
					}
				}
			}
		}
	}

	debugPrintMap(map, *bestPath);

	return bestScore;
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day17.txt");
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
				tmpBlock.map[Point{ i, row }] = values[i] - '0';
			}
			tmpBlock.width = values.size();
			tmpBlock.height = row + 1;
		}

		return tmpBlock;
	}) | std::ranges::to<std::vector>();

	auto map = maps[0];

	//auto bestScore = aStarSearch(map, { 0, 0 }, { map.width - 1, map.height - 1 });
	auto bestScore = aStarSearchV2(map, { 0, 0 }, { map.width - 1, map.height - 1 });

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	fmt::print("Processed 1: {}\n", bestScore);
	//fmt::print("Processed 2: {}\n", maxPoints);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
