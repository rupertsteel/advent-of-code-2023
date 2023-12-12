
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

std::array<Point, 4> plusOffsets{ {
		{ -1, 0},
	{ 1, 0},
	{ 0, 1},
	{ 0, -1}
} };

struct Run {
	std::string run;
	std::vector<int> arrangements;

	bool possiblyValid() const {
		// check upto the first '?' to see if we are valid
		// and that there is enough room to fit the rest in
		std::span<const int> remaninderToCheck = arrangements;
		std::string_view str = run;

		while (true) {
			// if the next remainderToCheck[0] values in str are ? or #, then remove remaininderToCheck[0]
			if (remaninderToCheck.empty()) {
				return true;
			}
			if (remaninderToCheck[0] > str.size()) {
				return false;
			}

			if (std::ranges::all_of(str | std::ranges::views::take(remaninderToCheck[0]), [](auto ch) {
				return ch == '#' || ch == '?';
			})) {
				if (str.size() == remaninderToCheck[0]) {
					str.substr(remaninderToCheck[0]);
					remaninderToCheck = remaninderToCheck.subspan(1);
					continue;
				}
				if (str.size() > remaninderToCheck[0] && str[remaninderToCheck[0]] != '#') {
					str.substr(remaninderToCheck[0] + 1);
					remaninderToCheck = remaninderToCheck.subspan(1);
					continue;
				}

			}

			str.remove_prefix(1);
		}
	}

	bool checkValid() const {
		auto vec = std::string_view{ run } | std::views::split("."sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); }) |
			std::views::transform([](auto sv) {
			return (int)sv.size();
				}) | std::views::filter([](auto val) {return val != 0; }) | std::ranges::to<std::vector>();

		return vec == arrangements;
	}

	uint64_t findArrangemnets() const {
		if (!possiblyValid()) {
			return 0;
		}

		if (run.find_first_of('?') == std::string::npos) {
			if (checkValid()) {
				return 1;
			} else {
				return 0;
			}
		}

		auto replaceIndex = run.find_first_of('?');

		uint64_t sum = 0;
		{
			Run tmpRun = *this;
			tmpRun.run[replaceIndex] = '.';
			sum += tmpRun.findArrangemnets();
		}
		{
			Run tmpRun = *this;
			tmpRun.run[replaceIndex] = '#';
			sum += tmpRun.findArrangemnets();
		}

		return sum;
	}
};

Run toRun(std::string_view sv) {
	auto runSv = sv.substr(0, sv.find_first_of(' '));

	sv.remove_prefix(sv.find_first_of(' ') + 1);

	Run run;
	run.run = runSv;

	while (!sv.empty()) {
		auto nextComma = sv.find_first_of(',');
		if (nextComma == std::string_view::npos) {
			std::string parseStr{ sv };
			run.arrangements.push_back(std::stoi(parseStr));
			return run;
		} else {
			std::string parseStr{ sv.substr(0, nextComma) };
			run.arrangements.push_back(std::stoi(parseStr));
			sv.remove_prefix(nextComma + 1);
		}
	}

	return run;
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day12.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	auto nonEmptyLines = lines | std::views::filter([](auto sv) { return !sv.empty(); });

	auto lineArrangements = nonEmptyLines | std::views::transform(toRun) | std::ranges::to<std::vector>();

	uint64_t totalArrangements = 0;
	for (auto line : lineArrangements) {
		totalArrangements += line.findArrangemnets();
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	//debugPrintMap(galaxies);

	fmt::print("Processed 1: {}\n", totalArrangements);
	//fmt::print("Processed 2: {}\n", countNotTouched);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
