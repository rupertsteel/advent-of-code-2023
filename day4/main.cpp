
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

struct Card {
	std::set<int> winingPoints;
	std::set<int> havePoints;
};

Card stringViewToCard(std::string_view sv) {
	sv = sv.substr(sv.find(':') + 1);

	Card card;

	while (!sv.empty() && sv[0] != '|') {
		if (sv[0] == ' ') {
			sv = sv.substr(1);
			continue;
		}

		auto numEnd = sv.find_first_of(' ');
		if (numEnd == std::string_view::npos) {
			numEnd = sv.size();
		}
		auto num = std::stoi(std::string{ sv.begin(), sv.begin() + numEnd });

		card.winingPoints.insert(num);

		sv = sv.substr(numEnd);
	}

	sv = sv.substr(1);

	while (!sv.empty() && sv[0] != '|') {
		if (sv[0] == ' ') {
			sv = sv.substr(1);
			continue;
		}

		auto numEnd = sv.find_first_of(' ');
		if (numEnd == std::string_view::npos) {
			numEnd = sv.size();
		}
		auto num = std::stoi(std::string{ sv.begin(), sv.begin() + numEnd });

		card.havePoints.insert(num);

		sv = sv.substr(numEnd);
	}

	return card;
}

int cardToScore(const Card& card) {

	std::vector<int> outputSet;

	auto overlap = std::ranges::set_intersection(card.winingPoints, card.havePoints, std::back_inserter(outputSet));

	return std::pow(2, outputSet.size() - 1);
}

int main(int argc, char* argv[]) {


	std::ifstream inputFile("inputs/day4.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	auto processed1 = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	auto processed2 = processed1 | std::views::filter([](auto sv) { return !sv.empty(); });

	auto scoreView =
		processed2 | std::views::transform(stringViewToCard) | std::views::transform(cardToScore);

	auto totalScore = std::accumulate(scoreView.begin(), scoreView.end(), 0);


	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;


	fmt::print("Processed 1: {}\n", totalScore);
	//fmt::print("Processed 2: {}\n", gearRatioSum);


	fmt::print("Took {}\n", dur);

	return 0;
}