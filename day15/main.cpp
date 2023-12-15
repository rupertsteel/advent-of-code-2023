
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

struct Lens {
	std::string name;
	int num;
};

struct Box {
	std::vector<Lens> lenses;
};

int boxHash(std::string_view sv) {
	int hashSum = 0;

	for (auto ch : sv) {
		if (ch == '\n') {
			continue;
		}

		hashSum += ch;
		hashSum *= 17;
		hashSum %= 256;
	}

	return hashSum;
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day15.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	input.erase(0, input.find_first_not_of("\t\n\v\f\r ")); // left trim
	input.erase(input.find_last_not_of("\t\n\v\f\r ") + 1); // right trim

	auto start = std::chrono::high_resolution_clock::now();

	//auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	//auto nonEmptyLines = lines | std::views::filter([](auto sv) { return !sv.empty(); });

	auto blocks = std::string_view{ input } | std::views::split(","sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });

	auto hashes = blocks | std::views::transform([](std::string_view sv) {
		return boxHash(sv);
	}) | std::ranges::to<std::vector>();

	auto sum = std::accumulate(hashes.begin(), hashes.end(), 0);


	std::array<Box, 256> boxes;

	for (auto lensInfo : blocks) {
		auto commandPos = lensInfo.find_first_of("=-");

		auto label = lensInfo.substr(0, commandPos);

		bool isAdding = lensInfo[commandPos] == '=';
		if (isAdding) {
			auto focus = std::stoi(std::string{ lensInfo.substr(commandPos + 1) });

			auto boxToAddTo = boxHash(label);

			auto& box = boxes[boxToAddTo];
			auto pos = std::ranges::find(box.lenses, label, [](auto& lens) { return lens.name; });

			if (pos != box.lenses.end()) {
				pos->num = focus;
			} else {
				box.lenses.push_back(Lens{ std::string{label}, focus });
			}
		} else {
			auto boxToAddTo = boxHash(label);

			auto& box = boxes[boxToAddTo];
			auto pos = std::ranges::find(box.lenses, label, [](auto& lens) { return lens.name; });
			if (pos != box.lenses.end()) {
				box.lenses.erase(pos);
			}
		}
	}

	uint64_t lensSum = 0;
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < boxes[i].lenses.size(); j++) {
			lensSum += ((i + 1) * (j + 1) * boxes[i].lenses[j].num);
		}
	}


	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	//debugPrintMap(galaxies);

	fmt::print("Processed 1: {}\n", sum);
	fmt::print("Processed 2: {}\n", lensSum);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
