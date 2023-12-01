
#include <fmt/format.h>
#include <iostream>
#include <fstream>
#include <ranges>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>

#include <fmt/ranges.h>
#include <fmt/chrono.h>

using namespace std::string_view_literals;

int main(int argc, char* argv[]) {


	std::ifstream inputFile("inputs/day1.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	auto processed1 = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	auto processed2 = processed1 | std::views::filter([](auto sv) { return !sv.empty(); });

	auto processed3 = processed2 | std::views::transform([](auto rng1) { return rng1 | std::views::filter([](auto d) { return std::isdigit(d); }); });

	auto processed4 = processed3 | std::views::transform([](auto rng) { return (rng | std::views::take(1) | std::ranges::to<std::string>() ) + (rng | std::views::reverse | std::views::take(1) | std::ranges::to<std::string>()); });

	auto processed5 = processed4 | std::views::transform([](auto rng1) { return std::stoi(rng1); });

	auto sumCalc = std::accumulate(processed5.begin(), processed5.end(), 0);

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;


	fmt::print("Processed 1: {}\n", sumCalc);


	fmt::print("Took {}\n", dur);

	return 0;
}