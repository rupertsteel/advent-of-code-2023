
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

	uint64_t scoreMap() {
		uint64_t totalScore = 0;

		for (auto val : map) {
			if (val.second == 'O') {
				totalScore += height - val.first.y;
			}
		}

		return totalScore;
	}
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

	maps[0].slideUp();
	uint64_t weight = maps[0].scoreMap();

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	//debugPrintMap(galaxies);

	fmt::print("Processed 1: {}\n", weight);
	//fmt::print("Processed 2: {}\n", totalArrangements2);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
