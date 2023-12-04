
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

int numMatching(const Card& card) {

	std::vector<int> outputSet;

	auto overlap = std::ranges::set_intersection(card.winingPoints, card.havePoints, std::back_inserter(outputSet));

	return outputSet.size();
}

int cardToScore(const Card& card) {

	int matches = numMatching(card);

	if (matches == 0) {
		return 0;
	}

	return std::pow(2, matches - 1);
}

std::pair<int, std::string_view> stringViewToCardNumStringView(std::string_view sv) {
	auto workingSv = sv;

	workingSv.remove_prefix(4);

	int numEnd = workingSv.find(':');

	int gameNum = std::stoi(std::string{ workingSv.begin(), workingSv.begin() + numEnd });

	return std::make_pair(gameNum, sv);
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

	auto cardNumCardView =
		processed2 | std::views::transform(stringViewToCardNumStringView) |
		std::views::transform([](std::pair<int, std::string_view> val) {
			return std::make_pair(val.first, stringViewToCard(val.second));
		})
		| std::views::transform([](std::pair<int, Card> val) {
			return std::make_pair(val.first, numMatching(val.second));
		});
	std::map<int, int> gameResults = cardNumCardView | std::ranges::to<std::map<int, int>>();

	std::map<int, int> numCard;
	for (const auto& card : gameResults) {
		numCard[card.first] = 1;
	}

	int numCardsProcessed = 0;
	for (int i = 1; i <= numCard.size(); i++) {
		auto sourceCards = numCard.at(i);

		numCardsProcessed += sourceCards;

		auto numToPushTo = gameResults.at(i);

		for (int j = 0; j < numToPushTo; j++) {
			if (numCard.contains(i + j + 1)) {
				numCard[i + j + 1] += sourceCards;
			}
		}
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;


	fmt::print("Processed 1: {}\n", totalScore);
	fmt::print("Processed 2: {}\n", numCardsProcessed);


	fmt::print("Took {}\n", dur);

	return 0;
}