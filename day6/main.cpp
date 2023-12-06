
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
#include <valarray>
#include <regex>
#include <set>

#include <fmt/ranges.h>
#include <fmt/chrono.h>

using namespace std::string_view_literals;

struct RaceInfo {
	uint64_t maxTimeMilliseconds;
	uint64_t recordDistanceMillimeters;
};

std::array<RaceInfo, 3> testRaces = {{
		{7, 9},
	{15, 40},
	{30, 200}
}};

std::array<RaceInfo, 4> realRaces = {{
	{44, 202},
	{82, 1076},
	{69, 1138},
	{81, 1458}
}};

std::array<RaceInfo, 1> part2testRaces = {{
	{ 71530, 940200 }
}};

std::array<RaceInfo, 1> part2realRaces = { {
	{ 44826981, 202107611381458 }
} };

uint64_t findNumberOfPassingRaces(RaceInfo race) {
	uint64_t numWays = 0;

	for (uint64_t len = 0; len <= race.maxTimeMilliseconds; len++) {
		auto distance = (race.maxTimeMilliseconds - len) * len;

		if (distance > race.recordDistanceMillimeters) {
			numWays++;
		}
	}

	return numWays;
}

int main(int argc, char* argv[]) {
	//std::ifstream inputFile("inputs/day5.txt");
	//std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	//auto& dataUsed = testRaces;
	auto& dataUsed = part2realRaces;

	uint64_t numWays = 1;

	for (const auto& race : dataUsed) {
		numWays *= findNumberOfPassingRaces(race);
	}



	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;


	fmt::print("Processed 1: {}\n", numWays);
	//fmt::print("Processed 2: {}\n", minRangeLocation);


	fmt::print("Took {}\n", dur);

	return 0;
}
