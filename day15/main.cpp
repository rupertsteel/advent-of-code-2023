
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

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day15.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	//auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	//auto nonEmptyLines = lines | std::views::filter([](auto sv) { return !sv.empty(); });

	auto blocks = std::string_view{ input } | std::views::split(","sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });

	


	auto hashes = blocks | std::views::transform([](std::string_view sv) {
		uint64_t hashSum = 0;

		for (auto ch : sv) {
			if (ch == '\n') {
				continue;
			}

			hashSum += ch;
			hashSum *= 17;
			hashSum %= 256;
		}

		return hashSum;
	}) | std::ranges::to<std::vector>();

	auto sum = std::accumulate(hashes.begin(), hashes.end(), 0);

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	//debugPrintMap(galaxies);

	fmt::print("Processed 1: {}\n", sum);
	//fmt::print("Processed 2: {}\n", billionCycleWeight);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
