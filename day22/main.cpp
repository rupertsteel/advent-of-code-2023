
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
	int64_t x;
	int64_t y;


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

struct Brick {
	Point3D bottomLowerLeft;
	Point3D deltaTopUpperRight;
	int id;
	bool fallen;
};

Brick parseBrick(std::string_view line) {
	std::regex r(R"((\d+),(\d+),(\d+)~(\d+),(\d+),(\d+))");

	using svmatch = std::match_results<std::string_view::const_iterator>;
	using svsub_match = std::sub_match<std::string_view::const_iterator>;

	svmatch match;

	std::regex_match(line.begin(), line.end(), match, r);

	auto x1 = std::stoi(std::string{ match[1].str()});
	auto y1 = std::stoi(std::string{ match[2].str()});
	auto z1 = std::stoi(std::string{ match[3].str()});

	auto x2 = std::stoi(std::string{ match[4].str()});
	auto y2 = std::stoi(std::string{ match[5].str()});
	auto z2 = std::stoi(std::string{ match[6].str()});

	return Brick{ {std::min(x1, x2), std::min(y1, y1), std::min(z1, z2)}, {std::max(x1, x2) - std::min(x1, x2), std::max(y1, y2) - std::min(y1, y2), std::max(z1, z2) - std::min(z1, z2)}, -1, false };
}

std::vector<Brick> fallBricks(std::vector<Brick> inputBricks) {
	std::vector<Brick> placedBricks;
	std::map<int, std::set<Point>> blockedPoints;

	while (!inputBricks.empty()) {
		auto minHeightBrick = std::ranges::min_element(inputBricks.begin(), inputBricks.end(), {}, [](auto& elem) { return elem.bottomLowerLeft.z; });

		// pull out the element
		auto processBrick = *minHeightBrick;
		inputBricks.erase(minHeightBrick);

		while (processBrick.bottomLowerLeft.z > 1) {
			// if we can we move down, move down

			if (processBrick.bottomLowerLeft.z == 1) {
				break;
			}

			if (!blockedPoints.contains(processBrick.bottomLowerLeft.z - 1)) {
				processBrick.bottomLowerLeft.z--;
				processBrick.fallen = true;
				continue;
			}

			auto checkHeight = processBrick.bottomLowerLeft.z - 1;
			auto& checkSet = blockedPoints.at(checkHeight);

			bool isSpaceFreeBelow = std::ranges::all_of(
				std::ranges::views::cartesian_product(
					std::ranges::views::iota(0, processBrick.deltaTopUpperRight.x + 1),
					std::ranges::views::iota(0, processBrick.deltaTopUpperRight.y + 1)
				) | std::ranges::views::transform([&](auto xy) {
					auto& [dx, dy] = xy;
					return !checkSet.contains(Point{ processBrick.bottomLowerLeft.x + dx, processBrick.bottomLowerLeft.y + dy });
			}), std::identity{});

			if (isSpaceFreeBelow) {
				processBrick.bottomLowerLeft.z--;
				processBrick.fallen = true;
				continue;
			} else {
				break;
			}
		}

		// place brick
		placedBricks.push_back(processBrick);

		// and set grid points
		for (int z = 0; z <= processBrick.deltaTopUpperRight.z; z++) {
			for (int x = 0; x <= processBrick.deltaTopUpperRight.x; x++) {
				for (int y = 0; y <= processBrick.deltaTopUpperRight.y; y++) {
					blockedPoints[processBrick.bottomLowerLeft.z + z].insert(Point{ processBrick.bottomLowerLeft.x + x, processBrick.bottomLowerLeft.y + y });
				}
			}
		}
	}

	return placedBricks;
}

uint64_t checkCountCanRemove(const std::vector<Brick>& bricks) {
	// for every brick, find out which bricks support it
	std::map<int, std::vector<int>> brickToSupportVec;
	std::map<int, std::vector<int>> supportToBrickVec;

	for (int i = 0; i < bricks.size(); i++) {
		brickToSupportVec[bricks[i].id];
		supportToBrickVec[bricks[i].id];

		for (int j = 0; j < bricks.size(); j++) {
			if (bricks[j].bottomLowerLeft.z + bricks[j].deltaTopUpperRight.z + 1 != bricks[i].bottomLowerLeft.z) {
				continue;
			}

			// j may be supporting i, so lets do a proper check
			bool support = false;

			for (int dix = 0; dix <= bricks[i].deltaTopUpperRight.x; dix++) {
				for (int diy = 0; diy <= bricks[i].deltaTopUpperRight.y; diy++) {
					for (int djx = 0; djx <= bricks[j].deltaTopUpperRight.x; djx++) {
						for (int djy = 0; djy <= bricks[j].deltaTopUpperRight.y; djy++) {

							auto ix = bricks[i].bottomLowerLeft.x + dix;
							auto iy = bricks[i].bottomLowerLeft.y + diy;

							auto jx = bricks[j].bottomLowerLeft.x + djx;
							auto jy = bricks[j].bottomLowerLeft.y + djy;

							if (ix == jx && iy == jy) {
								support = true;
							}
						}
					}
				}
			}

			if (support) {
				brickToSupportVec[bricks[i].id].push_back(bricks[j].id);
				supportToBrickVec[bricks[j].id].push_back(bricks[i].id);
			}
		}
	}

	std::set<int> canRemoveSet;

	for (auto& [supportId, supported] : supportToBrickVec) {
		// we check all the elements we support, if all of them have at least one other support, then this can be removed.
		bool canBeRemoved = true;

		for (auto& supportedBrick : supported) {
			if (brickToSupportVec[supportedBrick].size() < 2) {
				canBeRemoved = false;
			}
		}

		if (canBeRemoved) {
			canRemoveSet.insert(supportId);
		}
	}

	return canRemoveSet.size();
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day22.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); }) | std::views::filter([](auto sv) { return !sv.empty(); });

	auto bricks = lines | std::views::transform(parseBrick) | std::ranges::to<std::vector>();
	for (int i = 0;i < bricks.size(); i++) {
		bricks[i].id = i;
	}

	auto fallenBricks = fallBricks(bricks);

	auto canRemove = checkCountCanRemove(fallenBricks);

	int64_t numChain = 0;

	for (auto& brick : fallenBricks) {
		brick.fallen = false;
	}

	for (int i = 0; i < fallenBricks.size(); i++) {
		auto newVec = fallenBricks;
		newVec.erase(newVec.begin() + i);

		auto newFallenBricks = fallBricks(newVec);

		auto count_fallen = std::ranges::count_if(newFallenBricks, [](auto& b) {return b.fallen; });

		numChain += count_fallen;
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	fmt::print("Processed 1: {}\n", canRemove);
	fmt::print("Processed 2: {}\n", numChain);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
