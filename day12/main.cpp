
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

	void expand() {
		auto newStr = run + "?" + run + "?" + run;
		std::vector<int> newArrangements;
		for (int i = 0; i < 3; i++) {
			for (auto val : arrangements) {
				newArrangements.push_back(val);
			}
		}

		run = newStr;
		arrangements = newArrangements;
	}

	uint64_t findArrangements2Internal(std::span<const int> remainingItems, std::string processStr, std::string_view remainStr, const std::span<const int> allItems) {
		// move any empty spaces to the processStr
		while (!remainStr.empty() && remainStr[0] == '.') {
			processStr.push_back('.');
			remainStr.remove_prefix(1);
		}

		if (remainStr.empty() && !remainingItems.empty()) {
			return 0;
		}
		if (remainingItems.empty() && std::ranges::all_of(remainStr, [](auto ch) {
			return ch == '.' || ch == '?';
		})) {
			return 1;
		}

		auto checkNextItemWorks = [&]() {
			if (remainStr.size() > remainingItems[0]) {
				return std::ranges::all_of(remainStr | std::views::take(remainingItems[0]), [](auto ch) {
					return ch == '#' || ch == '?';
					}) && (
						remainStr[remainingItems[0]] == '#'
						|| remainStr[remainingItems[0]] == '?'
						);
			} else {
				return std::ranges::all_of(remainStr | std::views::take(remainingItems[0]), [](auto ch) {
					return ch == '#' || ch == '?';
					});
			}
		};

		while (remainStr.size() > remainingItems[0]) {
			if (!checkNextItemWorks()) {
				// it currently doesn't work, progress
				if (remainStr[0] == '#') {
					// it completely doesn't work
					return 0;
				}

				processStr.push_back('.');
				remainStr.remove_prefix(1);
				continue;
			}

			uint64_t pathSum = 0;

			// it works here, do a match, then a non match
			{
				if (remainStr.size() > remainingItems[0]) {
					auto processStr2 = processStr;
					for (int i = 0; i < remainingItems[0]; i++) {
						processStr2.push_back('#');
					}
					processStr2.push_back('.');

					auto remainStr2 = remainStr.substr(remainingItems[0] + 1);

					auto remainingItems2 = remainingItems.subspan(1);

					pathSum += findArrangements2Internal(remainingItems2, processStr2, remainStr2, allItems);
				} else {
					auto processStr2 = processStr;
					for (int i = 0; i < remainingItems[0]; i++) {
						processStr2.push_back('#');
					}

					auto remainStr2 = remainStr.substr(remainingItems[0]);

					auto remainingItems2 = remainingItems.subspan(1);

					pathSum += findArrangements2Internal(remainingItems2, processStr2, remainStr2, allItems);
				}
			}
			{
				auto processStr2 = processStr;
				processStr2.push_back('.');

				auto remainStr2 = remainStr.substr(1);

				pathSum += findArrangements2Internal(remainingItems, processStr2, remainStr2, allItems);
			}

			return pathSum;
		}

		return 0;
	}

	uint64_t findArrangements2() {
		std::span<int> remainingItems = arrangements;
		std::string processStr = "";
		std::string_view remainStr = run;
		std::span<int> allItems = arrangements;

		return findArrangements2Internal(remainingItems, processStr, remainStr, allItems);
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
	/*uint64_t totalArrangements2 = 0;
	int numRows = 0;
	for (auto line : lineArrangements) {
		line.expand();

		//totalArrangements += line.findArrangemnets();
		totalArrangements2 += line.findArrangements2();

		++numRows;

		fmt::print("Processed {}/1000\n", numRows);
	}*/


	std::atomic<int> numProcesed = 0;

	uint64_t totalArrangements2 = std::transform_reduce(std::execution::par_unseq, lineArrangements.begin(), lineArrangements.end(), 0ull, std::plus<>(), [&](auto line) {
		line.expand();

		auto val = line.findArrangements2();

		auto processAmount = numProcesed.fetch_add(1);
		++processAmount;

		fmt::print("Processed {}/1000\n", processAmount);

		return val;
	});

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	//debugPrintMap(galaxies);

	fmt::print("Processed 1: {}\n", totalArrangements);
	fmt::print("Processed 2: {}\n", totalArrangements2);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
