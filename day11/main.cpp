
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

void debugPrintMap(const std::set<Point>& galaxies) {
	int maxX = std::ranges::max(galaxies, {}, [](auto& elem) {
		return elem.x;
	}).x;
	int maxY = std::ranges::max(galaxies, {}, [](auto& elem) {
		return elem.y;
	}).y;

	std::string returnStr;

	for (int y = 0; y <= maxY; ++y) {
		for (int x = 0; x <= maxX; x++) {
			if (galaxies.contains(Point{x, y})) {
				returnStr.push_back('#');
			} else {
				returnStr.push_back('.');
			}
		}

		returnStr.push_back('\n');
	}

	fmt::println("{}", returnStr);
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day11.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	auto nonEmptyLines = lines | std::views::filter([](auto sv) { return !sv.empty(); });

	std::set<Point> galaxies;
	std::set<int> usedRows;
	std::set<int> usedColumns;

	int maxX = 0;

	int y = 0;
	for (const auto& line : nonEmptyLines) {
		for (int x = 0; x < line.size(); x++) {
			if (line[x] == '#') {
				galaxies.insert(Point{ x, y });
				usedRows.insert(y);
				usedColumns.insert(x);
				maxX = std::max(maxX, x);
			}
		}

		++y;
	}
	
	std::vector<int> emptyRows;
	std::vector<int> emptyColumns;

	for (int removeX = 0; removeX <= maxX; ++removeX) {
		if (!usedColumns.contains(removeX)) {
			emptyColumns.push_back(removeX);
		}
	}
	for (int removeY = 0; removeY <= y; ++removeY) {
		if (!usedRows.contains(removeY)) {
			emptyRows.push_back(removeY);
		}
	}

	std::ranges::reverse(emptyRows);
	std::ranges::reverse(emptyColumns);

	for (auto expandRow : emptyRows) {
		std::set<Point> newGalaxies;

		for (auto it = galaxies.begin(); it != galaxies.end();) {
			if (it->y > expandRow) {
				newGalaxies.insert(Point{ it->x, it->y + 999'999 });
				it = galaxies.erase(it);
			} else {
				++it;
			}
		}

		galaxies.merge(newGalaxies);
	}
	for (auto expandColumn : emptyColumns) {
		std::set<Point> newGalaxies;

		for (auto it = galaxies.begin(); it != galaxies.end();) {
			if (it->x > expandColumn) {
				newGalaxies.insert(Point{ it->x + 999'999, it->y });
				it = galaxies.erase(it);
			} else {
				++it;
			}
		}

		galaxies.merge(newGalaxies);
	}

	uint64_t totalDistance = 0;

	for (auto it1 = galaxies.begin(); it1 != galaxies.end(); ++it1) {
		for (auto it2 = std::next(it1); it2 != galaxies.end(); ++it2) {
			auto distanceX = std::abs(it2->x - it1->x);
			auto distanceY = std::abs(it2->y - it1->y);

			totalDistance += distanceX;
			totalDistance += distanceY;
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	//debugPrintMap(galaxies);

	fmt::print("Processed 1: {}\n", totalDistance);
	//fmt::print("Processed 2: {}\n", countNotTouched);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
