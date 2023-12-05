
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

struct Mapping {
	uint64_t inputStart;
	uint64_t outputStart;
	uint64_t length;
};

struct Mapper {
	std::vector<Mapping> mappings;

	void parseLine(std::string_view line) {
		auto input = std::stoull(std::string{ line.begin(), line.begin() + line.find_first_of(' ') });

		line = line.substr(line.find_first_of(' ') + 1);

		auto output = std::stoull(std::string{ line.begin(), line.begin() + line.find_first_of(' ') });
		line = line.substr(line.find_first_of(' ') + 1);

		auto range = std::stoull(std::string{ line });

		mappings.push_back(Mapping{ output, input, range });
	}

	uint64_t map(uint64_t input) {
		for (auto& mapping : mappings) {
			if (input >= mapping.inputStart && input < (mapping.inputStart + mapping.length)) {
				return input - mapping.inputStart + mapping.outputStart;
			}
		}

		return input;
	}
};

std::vector<uint64_t> parseSeeds(std::string_view line) {
	line.remove_prefix(7);

	std::vector<uint64_t> output;

	while (line.find_first_of(' ') != std::string_view::npos) {

		auto val = std::stoull(std::string{ line.begin(), line.begin() + line.find_first_of(' ') });
		output.push_back(val);

		line = line.substr(line.find_first_of(' ') + 1);
	}

	auto val = std::stoull(std::string{ line });

	output.push_back(val);

	return output;
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day5.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	std::vector<uint64_t> seeds;

	Mapper seedToSoil;
	Mapper soilToFertilizer;
	Mapper fertilizerToWater;
	Mapper waterToLight;
	Mapper lightToTemperature;
	Mapper temperatureToHumidity;
	Mapper humidityToLocation;

	auto processed1 = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	auto lines = processed1 | std::ranges::to<std::vector>();

	Mapper* readIntoMapper = &seedToSoil;

	for (int i = 0; i < lines.size(); i++) {
		if (lines[i].starts_with("seeds:")) {
			seeds = parseSeeds(lines[i]);
			continue;
		}

		if (lines[i].starts_with("seed-to-soil map")) {
			readIntoMapper = &seedToSoil;
			continue;
		}
		if (lines[i].starts_with("soil-to-fertilizer map")) {
			readIntoMapper = &soilToFertilizer;
			continue;
		}
		if (lines[i].starts_with("fertilizer-to-water map")) {
			readIntoMapper = &fertilizerToWater;
			continue;
		}
		if (lines[i].starts_with("water-to-light map")) {
			readIntoMapper = &waterToLight;
			continue;
		}
		if (lines[i].starts_with("light-to-temperature map")) {
			readIntoMapper = &lightToTemperature;
			continue;
		}
		if (lines[i].starts_with("temperature-to-humidity map")) {
			readIntoMapper = &temperatureToHumidity;
			continue;
		}
		if (lines[i].starts_with("humidity-to-location map")) {
			readIntoMapper = &humidityToLocation;
			continue;
		}

		if (lines[i].empty()) {
			continue;
		}

		readIntoMapper->parseLine(lines[i]);
	}

	uint64_t minLocation = std::numeric_limits<uint64_t>::max();

	for (auto seed : seeds) {
		auto soil = seedToSoil.map(seed);
		auto fertilizer = soilToFertilizer.map(soil);
		auto water = fertilizerToWater.map(fertilizer);
		auto light = waterToLight.map(water);
		auto temperature = lightToTemperature.map(light);
		auto humidity = temperatureToHumidity.map(temperature);
		auto location = humidityToLocation.map(humidity);

		minLocation = std::min(location, minLocation);
	}


	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;


	fmt::print("Processed 1: {}\n", minLocation);
	//fmt::print("Processed 2: {}\n", numCardsProcessed);


	fmt::print("Took {}\n", dur);

	return 0;
}
