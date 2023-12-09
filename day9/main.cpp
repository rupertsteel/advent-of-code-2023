
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

#include <fmt/ranges.h>
#include <fmt/chrono.h>

using namespace std::string_view_literals;


std::vector<int64_t> predictVec(std::vector<long long> vec) {
	// if the entire vector is the same value, return vector with one extra value
	if (std::ranges::all_of(vec, [&](auto val) {
		return val == 0;
	})) {
		vec.push_back(0);
		return vec;
	}

	// create delta vec
	std::vector<long long> deltaVec;
	for (int i = 1; i < vec.size(); i++) {
		deltaVec.push_back(vec[i] - vec[i - 1]);
	}

	auto deltaPredictVec = predictVec(deltaVec);

	// add the delta value
	vec.push_back(vec[vec.size() - 1] + deltaPredictVec[deltaPredictVec.size() - 1]);

	return vec;
}

int64_t predictLineValue(const std::vector<long long>& vec) {

	auto vector = predictVec(vec);
	return vector[vector.size() - 1];
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day9.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	auto nonEmptyLines = lines | std::views::filter([](auto sv) { return !sv.empty(); });

	auto parsedLines = nonEmptyLines | std::views::transform([](std::string_view sv) -> std::vector<std::int64_t> {
		return sv | std::views::split(" "sv) | std::views::transform([](auto rng) { return std::string{ rng.begin(), rng.end() }; }) | std::views::transform([](std::string str) {
			return std::stoll(str);
		}) | std::ranges::to<std::vector>();
	}) | std::ranges::to<std::vector>();

	int64_t predictSum = 0;

	for (auto& vec : parsedLines) {
		auto val = predictLineValue(vec);
		fmt::println("Vec: {}", val);

		predictSum += val;
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;


	fmt::print("Processed 1: {}\n", predictSum);
	//fmt::print("Processed 2: {}\n", ghostEndStep);


	fmt::print("Took {}\n", dur);

	return 0;
}
