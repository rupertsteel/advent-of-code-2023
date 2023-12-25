
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

struct Point3D {
	int64_t x;
	int64_t y;
	int64_t z;

	std::strong_ordering operator<=>(const Point3D&) const = default;
	bool operator==(const Point3D&) const = default;


	Point3D operator+(const Point3D& other) const {
		return Point3D{ x + other.x, y + other.y, z + other.z };
	}
};

struct Hail {
	Point3D startPoint;
	Point3D velocity;
};

Hail parseHail(std::string_view line) {
	std::regex r(R"((-?\d+),\s+(-?\d+),\s+(-?\d+)\s+@\s+(-?\d+),\s+(-?\d+),\s+(-?\d+))");

	using svmatch = std::match_results<std::string_view::const_iterator>;
	using svsub_match = std::sub_match<std::string_view::const_iterator>;

	svmatch match;

	std::regex_match(line.begin(), line.end(), match, r);

	auto x1 = std::stoll(std::string{ match[1].str() });
	auto y1 = std::stoll(std::string{ match[2].str() });
	auto z1 = std::stoll(std::string{ match[3].str() });

	auto x2 = std::stoll(std::string{ match[4].str() });
	auto y2 = std::stoll(std::string{ match[5].str() });
	auto z2 = std::stoll(std::string{ match[6].str() });

	return Hail{ {x1, y1, z1}, {x2, y2, z2} };
}

int64_t countIntersections2d(const std::vector<Hail>& hails, int64_t minBound, int64_t maxBound) {
	// intersect point
	// i == a.start + a.delta * time
	// i == b.start + b.delta * time
	// a.start + a.delta * time == b.start + b.delta * time
	// note, t can be fractional
	// a.start + a.delta * time - b.start == b.delta * time
	// a.start - b.start == b.delta * time - a.delta * time
	// a.start - b.start == (b.delta - a.delta) * time

	int64_t intersectionCount = 0;

	for (int i = 0; i < hails.size(); i++) {
		for (int j = i + 1; j < hails.size(); j++) {
			auto& hailI = hails[i];
			auto& hailJ = hails[j];

			auto xd1 = static_cast<double>(hailI.velocity.x);
			auto xd2 = static_cast<double>(hailJ.velocity.x);
			auto y1 = static_cast<double>(hailI.startPoint.y);
			auto y2 = static_cast<double>(hailJ.startPoint.y);
			auto x1 = static_cast<double>(hailI.startPoint.x);
			auto yd2 = static_cast<double>(hailJ.velocity.y);
			auto yd1 = static_cast<double>(hailI.velocity.y);
			auto x2 = static_cast<double>(hailJ.startPoint.x);


			double tI = -((xd2 * y1 - xd2 * y2 - x1 * yd2 + x2 * yd2) / (xd2 * yd1 - xd1 * yd2));
			double tJ = -((xd1 * y1 - xd1 * y2 - x1 * yd1 + x2 * yd1) / (xd2 * yd1 - xd1 * yd2));


			auto iIntersectX = hailI.startPoint.x + tI * hailI.velocity.x;
			auto jIntersectX = hailJ.startPoint.x + tJ * hailJ.velocity.x;

			auto iIntersectY = hailI.startPoint.y + tI * hailI.velocity.y;
			auto jIntersectY = hailJ.startPoint.y + tJ * hailJ.velocity.y;

			if (tI > 0 && tJ > 0) {
				if (iIntersectX > minBound && iIntersectX < maxBound && iIntersectY > minBound && iIntersectY < maxBound) {
					++intersectionCount;
				}
			}
		}
	}

	return intersectionCount;
}

void printMathematicaSolveExpression(const std::vector<Hail>& hails) {
	for (int i = 0; i < hails.size(); i++) {
		fmt::print("rx + rdx * t{} == {} + {} * t{},", i, hails[i].startPoint.x, hails[i].velocity.x, i);
		fmt::print("ry + rdy * t{} == {} + {} * t{},", i, hails[i].startPoint.y, hails[i].velocity.y, i);
		fmt::print("rz + rdz * t{} == {} + {} * t{},", i, hails[i].startPoint.z, hails[i].velocity.z, i);
	}
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day24.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	//auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	//auto nonEmptyLines = lines | std::views::filter([](auto sv) { return !sv.empty(); });

	auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); }) | std::views::filter([](auto sv) { return !sv.empty(); });

	auto hailstones = lines | std::views::transform(parseHail) | std::ranges::to<std::vector>();

	// test
	//auto intersections = countIntersections2d(hailstones, 7, 27);
	// actual
	auto intersections = countIntersections2d(hailstones, 200000000000000, 400000000000000);

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	//debugPrintMapPath(map, *longestPath);

	printMathematicaSolveExpression(hailstones);

	fmt::print("Processed 1: {}\n", intersections);
	//fmt::print("Processed 2: {}\n", longestPath2->currentSteps);




	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
