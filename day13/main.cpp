
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

	//int findReflectionNumber(std::optional<int> reflectSkipX, std::optional<int> reflectSkipY) const {
	int findReflectionNumber(const std::set<int>& reflectSkipX, const std::set<int>& reflectSkipY) const {
		auto checkXReflection = [&](int x) {
			// y is the first line of reflection down
			for (int xOff = 0; xOff < width; xOff++) {
				auto compareRow1 = x - (xOff + 1);
				auto compareRow2 = x + xOff;
				if (compareRow1 < 0 || compareRow2 >= width) {
					continue;
				}

				for (int y = 0; y < height; y++) {
					auto cmp = map.at(Point{ compareRow1, y }) == map.at(Point{ compareRow2, y });
					if (!cmp) {
						return false;
					}
				}
			}

			return true;
		};

		auto checkYReflection = [&](int y) {
			// y is the first line of reflection down
			for (int yOff = 0; yOff < height; yOff++) {
				auto compareRow1 = y - (yOff + 1);
				auto compareRow2 = y + yOff;
				if (compareRow1 < 0 || compareRow2 >= height) {
					continue;
				}

				for (int x = 0; x < width; x++) {
					auto cmp = map.at(Point{ x, compareRow1 }) == map.at(Point{ x, compareRow2 });
					if (!cmp) {
						return false;
					}
				}
			}

			return true;
		};

		for (int x = 1; x < width; x++) {
			// if (reflectSkipX == x) {
			// 	continue;
			// }
			if (reflectSkipX.contains(x)) {
				continue;
			}

			if (checkXReflection(x)) {
				return x;
			}
		}

		for (int y = 1; y < height; y++) {
			// if (reflectSkipY == y) {
			// 	continue;
			// }
			if (reflectSkipY.contains(y)) {
				continue;
			}

			if (checkYReflection(y)) {
				// we have a valid y reflection
				return y * 100;
			}
		}

		return 0;
	}
};


int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day13.txt");
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

	int mapNum = 0;

	auto mapNumbers = std::transform_reduce(maps.begin(), maps.end(), 0, std::plus<>{}, [&mapNum](auto& map) {
		//auto val1 = map.findReflectionNumber(std::nullopt, std::nullopt);


		std::set<int> reflectSkipX;
		std::set<int> reflectSkipY;

		while (true) {
			auto val = map.findReflectionNumber(reflectSkipX, reflectSkipY);

			if (val == 0) {
				break;
			}

			if (val >= 100) {
				reflectSkipY.insert(val / 100);
			} else {
				reflectSkipX.insert(val);
			}
		}

		// if (val1 > 100) {
		// 	reflectSkipY = val1 / 100;
		// } else {
		// 	reflectSkipX = val1;
		// }
		//
		// auto val2 = map.findReflectionNumber(std::nullopt, std::nullopt);

		// if (val2 != 0) {
		// 	fmt::println("This is the problem");
		// }
		//
		// if (val2 > 100) {
		// 	reflectSkipY = val2 / 100;
		// } else {
		// 	reflectSkipX = val2;
		// }

		for (int y = 0; y < map.height; y++) {
			for (int x = 0; x < map.width; x++) {
				if (map.map[Point{x, y}] == '#') {
					map.map[Point{ x, y }] = '.';
				} else {
					map.map[Point{ x, y }] = '#';
				}
		
				auto val = map.findReflectionNumber(reflectSkipX, reflectSkipY);
		
				if (map.map[Point{ x, y }] == '#') {
					map.map[Point{ x, y }] = '.';
				} else {
					map.map[Point{ x, y }] = '#';
				}
		
				if (val != 0) {
					fmt::println("Map {}: {}", mapNum++, val);
					return val;
				}
			}
		}

		throw std::runtime_error("Did not find end");
	});

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	//debugPrintMap(galaxies);

	fmt::print("Processed 1: {}\n", mapNumbers);
	//fmt::print("Processed 2: {}\n", totalArrangements2);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
