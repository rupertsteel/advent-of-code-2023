
#include <fmt/format.h>
#include <iostream>
#include <fstream>
#include <ranges>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <valarray>
#include <regex>

#include <fmt/ranges.h>
#include <fmt/chrono.h>

using namespace std::string_view_literals;

struct GameWorking {
	int gameId;
	std::vector<std::valarray<int>> rounds;
};

int main(int argc, char* argv[]) {


	std::ifstream inputFile("inputs/day2.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	auto processed1 = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	auto processed2 = processed1 | std::views::filter([](auto sv) { return !sv.empty(); });

	auto processed3 = processed2 | std::views::transform([](std::string_view sv) {
		GameWorking game;

		std::regex parseRegex{ R"(^Game (\d+):(.*)$)" };

		std::string str{ sv };
		std::smatch matchResults;

		auto match = std::regex_match(str, matchResults, parseRegex);

		game.gameId = std::stoi(matchResults[1]);
		std::string rounds = matchResults[2];

		auto roundsValue = std::string_view{ rounds } | std::views::split(";"sv) | std::views::transform([](auto rng) {
			return std::string_view(rng.begin(), rng.end());
		}) | std::views::transform([](auto rng) {
			auto tempRng = rng | std::views::split(","sv) | std::views::transform([](auto rng) {
				return std::string_view(rng.begin(), rng.end());
			}) | std::views::transform([](auto rng) -> std::valarray<int> {
				std::regex groupParseRegex{ R"(^\s*(\d+) (\w+)\s*$)" };
				std::string str2{ rng };
				std::smatch matchResults2;

				auto match2 = std::regex_match(str2, matchResults2, groupParseRegex);
				if (!match2) {
					throw std::runtime_error("group parse failed");
				}

				auto amount = std::stoi(matchResults2[1]);
				auto color = matchResults2[2];
				if (color == "red") {
					return { amount, 0, 0 };
				} else if (color == "green") {
					return { 0, amount, 0 };
				} else {
					return { 0, 0, amount };
				}
			});

			return std::accumulate(tempRng.begin(), tempRng.end(), std::valarray<int>{ 0, 0, 0 });
		});

		game.rounds = roundsValue | std::ranges::to<std::vector<std::valarray<int>>>();

		//return rng1 | std::views::filter([](auto d) { return std::isdigit(d); });
		return game;
	});

	auto processed4 = processed3 | std::views::filter([](const GameWorking& game) {
		for (auto& round : game.rounds) {
			if (round[0] > 12 || round[1] > 13 || round[2] > 14) {
				return false;
			}

		}

		return true;
	});

	auto processed5 = processed4 | std::views::transform([](auto rng1) { return rng1.gameId; });

	auto sumCalc = std::accumulate(processed5.begin(), processed5.end(), 0);

	auto minCubes = processed3 | std::views::transform([](const GameWorking& game) {
		return std::reduce(game.rounds.begin(), game.rounds.end(), std::valarray{ 0,0,0 }, [](const auto& left, const auto& right) {
			return std::valarray{ std::max(left[0], right[0]), std::max(left[1], right[1]), std::max(left[2], right[2]) };
		});
	});

	auto powers = minCubes | std::views::transform([](const std::valarray<int>& arr) {
		return arr[0] * arr[1] * arr[2];
	});

	auto powerAccumulate = std::accumulate(powers.begin(), powers.end(), 0);

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;


	fmt::print("Processed 1: {}\n", sumCalc);
	fmt::print("Processed 2: {}\n", powerAccumulate);


	fmt::print("Took {}\n", dur);

	return 0;
}