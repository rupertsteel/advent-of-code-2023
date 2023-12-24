
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
#include <semaphore>
#include <stack>
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

std::array<Point, 4> plusOffsets{ {
		{ -1, 0},
	{ 1, 0},
	{ 0, 1},
	{ 0, -1}
} };

std::vector<std::shared_ptr<std::set<Point>>> toDeallocateList;
std::mutex toDeallocateListMutex;
std::counting_semaphore<> toDeallocateListSemaphore{ 0 };

struct FastUpdatePointSet {
	std::shared_ptr<std::set<Point>> commonElements = std::make_unique<std::set<Point>>();

	std::vector<Point> recentElements;

	FastUpdatePointSet() = default;

	FastUpdatePointSet(const FastUpdatePointSet&) = default;
	FastUpdatePointSet(FastUpdatePointSet&&) = default;
	FastUpdatePointSet& operator=(const FastUpdatePointSet&) = default;
	FastUpdatePointSet& operator=(FastUpdatePointSet&&) = default;

	~FastUpdatePointSet() {
		{
			std::scoped_lock<std::mutex> local(toDeallocateListMutex);
			toDeallocateList.push_back(std::move(commonElements));
			toDeallocateListSemaphore.release();
		}
	}

	void rebalanceIfRequired() {
		if (recentElements.size() > 50) {
			if (commonElements.use_count() == 1) {
				for (auto& recentElem : recentElements) {
					commonElements->insert(recentElem);
				}

				recentElements.clear();
				return;
			}
			auto newCommonSet = std::make_shared<std::set<Point>>(*commonElements);

			for (auto& recentElem : recentElements) {
				newCommonSet->insert(recentElem);
			}

			commonElements = newCommonSet;
			recentElements.clear();
		}
	}

	void insert(Point point) {
		recentElements.push_back(point);

		rebalanceIfRequired();
	}

	bool contains(Point point) const {
		if (commonElements->contains(point)) {
			return true;
		}

		return std::ranges::any_of(recentElements, [&](auto elem) {
			return elem == point;
		});
	}
};

struct Path {
	Point currentPoint;

	int currentSteps;

	//std::set<Point> exploredPoints;
	//FastUpdatePointSet exploredPoints;
	std::vector<Point> exploredPoints;
};

std::vector<Path> pathSearch(const Block& block, bool hills) {
	// find our starting square
	Point startingPoint{ 0, 0 };
	for (int i = 0; i < block.width; i++) {
		if (block.map.at(Point{ i, 0 }) == '.') {
			startingPoint.x = i;
		}
	}

	Point endingPoint{ 0, block.height - 1 };
	for (int i = 0; i < block.width; i++) {
		if (block.map.at(Point{ i, block.height - 1 }) == '.') {
			endingPoint.x = i;
		}
	}

	//Path startingPath{ startingPoint, 0, {startingPoint} };
	Path startingPath;
	startingPath.currentPoint = startingPoint;
	startingPath.currentSteps = 0;
	//startingPath.exploredPoints.insert(startingPoint);
	startingPath.exploredPoints.push_back(startingPoint);

	std::stack<Path> pathsToSearch;
	pathsToSearch.push(startingPath);

	std::vector<Path> endPaths;

	uint64_t it = 0;

	auto lastPrint = std::chrono::high_resolution_clock::now();

	while (!pathsToSearch.empty()) {
		++it;
		if (it % 10'000 == 0) {
			auto printTime = std::chrono::high_resolution_clock::now();
			auto dur = printTime - lastPrint;
			auto rate = 10000.0f / std::chrono::duration_cast<std::chrono::duration<float>>(dur).count();

			lastPrint = printTime;

			fmt::println("Processing it {}, depth {}, {} it/s", it, pathsToSearch.size(), rate);
		}

		// grab the path
		auto processPath = pathsToSearch.top();
		pathsToSearch.pop();

		if (processPath.currentPoint == endingPoint) {
			endPaths.push_back(processPath);
			continue;
		}

		for (int i = 0; i < 4; i++) {
			auto proposedPoint = processPath.currentPoint + plusOffsets[i];

			if (proposedPoint.x < 0 || proposedPoint.y < 0 || proposedPoint.x >= block.width || proposedPoint.y >= block.height) {
				continue;
			}

			if (block.map.at(proposedPoint) == '#') {
				continue;
			}

			//if (processPath.exploredPoints.contains(proposedPoint)) {
			if (std::ranges::contains(processPath.exploredPoints, proposedPoint)) {
				continue;
			}

			if (hills) {
				if (
					(block.map.at(processPath.currentPoint) == '<' && i != 0) ||
					(block.map.at(processPath.currentPoint) == '>' && i != 1) ||
					(block.map.at(processPath.currentPoint) == 'v' && i != 2) ||
					(block.map.at(processPath.currentPoint) == '^' && i != 3)
					) {
					continue;
				}
			}

			auto updatePath = processPath;
			updatePath.currentPoint = proposedPoint;
			updatePath.currentSteps++;
			//updatePath.exploredPoints.insert(updatePath.currentPoint);
			updatePath.exploredPoints.push_back(updatePath.currentPoint);

			pathsToSearch.push(updatePath);
		}
	}

	return endPaths;
}

void debugPrintMapPath(const Block& map, const Path& path) {
	std::string printStr;

	for (int y = 0; y < map.height; y++) {
		for (int x = 0; x < map.width; x++) {
			//if (path.exploredPoints.contains(Point{x, y})) {
			if (std::ranges::contains(path.exploredPoints, Point{ x, y })) {
				printStr += 'O';
			} else {
				printStr += map.map.at(Point{ x, y });
			}
		}

		printStr += '\n';
	}

	fmt::println("{}", printStr);
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day23.txt");
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

	std::atomic_bool cleanupThreadShouldRun{ true };

	std::jthread cleanupThread{ [&]() {
		while (cleanupThreadShouldRun.load(std::memory_order::release)) {
			bool acquired = toDeallocateListSemaphore.try_acquire_for(std::chrono::seconds{ 5 });

			if (!acquired) {
				continue;
			}

			std::shared_ptr<std::set<Point>> toDeallocateItem;

			{
				std::scoped_lock lock{ toDeallocateListMutex };
				toDeallocateItem = std::move(toDeallocateList.back());
				toDeallocateList.pop_back();
			}
		}
	} };

	auto paths = pathSearch(map, true);
	auto longestPath = std::ranges::max_element(paths, {}, [](auto& elem) {
		return elem.currentSteps;
	});

	auto paths2 = pathSearch(map, false);
	auto longestPath2 = std::ranges::max_element(paths2, {}, [](auto& elem) {
		return elem.currentSteps;
	});

	cleanupThreadShouldRun = false;

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	debugPrintMapPath(map, *longestPath);

	fmt::print("Processed 1: {}\n", longestPath->currentSteps);
	fmt::print("Processed 2: {}\n", longestPath2->currentSteps);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
