
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

struct HandBid {
	std::string hand;
	int bid;
};

HandBid lineToHandBid(std::string_view sv) {
	HandBid working;

	working.hand = sv.substr(0, 5);

	sv.remove_prefix(6);

	for (auto& ch : working.hand) {
		// replace chars so we can use ascii order
		if (ch == 'T') {
			ch = '<';
		}

		if (ch == 'K') {
			ch = 'U';
		}
		if (ch == 'A') {
			ch = 'Z';
		}

		if (ch == 'J') {
			ch = '*';
		}
	}

	std::string tempStr{ sv };
	working.bid = std::stoi(tempStr);

	return working;
}

int findHandTier(const std::string& hand) {
	std::map<char, int> values;

	for (auto val : hand) {
		values[val]++;
	}

	if (values.size() == 1) {
		return 10; // five of a kind
	}
	if (values.size() == 2) {
		for (auto& [key, amount] : values) {
			if (amount == 4) {
				return 9; // four of a kind, higher set
			}
			if (amount == 2) {
				return 8; // full house, lower set
			}
		}
	}
	if (values.size() == 3) {
		for (auto& [key, amount] : values) {
			if (amount == 3) {
				return 7; // three of a kind, highest set
			}
			if (amount == 2) {
				return 6; // two pair, we can't overlap with full house
			}
		}
	}
	if (values.size() == 4) {
		return 5; // two pair
	}
	return 4; // high card
}

int findJokerHandTier(const std::string& hand) {
	// if the hand doesn't have a joker, pass to the underlying function
	if (std::ranges::none_of(hand, [](char val) {return val == '*';})) {
		return findHandTier(hand);
	}

	// we have a joker, replace it with every other type
	auto jokerPos = hand.find('*');

	int highestVal = 0;

	std::array replaceChars = { '2', '3', '4', '5', '6', '7', '8', '9', '<', 'Q', 'U', 'Z' };
	for (auto ch : replaceChars) {
		auto newStr = hand;
		newStr[jokerPos] = ch;

		auto checkVal = findJokerHandTier(newStr);

		highestVal = std::max(checkVal, highestVal);
	}

	return highestVal;
}

std::strong_ordering handBidSort(const HandBid& left, const HandBid& right) {
	auto tierLeft = findJokerHandTier(left.hand);
	auto tierRight = findJokerHandTier(right.hand);

	auto compare = tierLeft <=> tierRight;

	if (compare != std::strong_ordering::equal) {
		return compare;
	}

	return left.hand <=> right.hand;
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day7.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	auto nonEmptyLines = lines | std::views::filter([](auto sv) { return !sv.empty(); });

	auto handBids = nonEmptyLines | std::views::transform(lineToHandBid) | std::ranges::to<std::vector>();

	std::ranges::sort(handBids, [](const HandBid& left, const HandBid& right) { return handBidSort(left, right) < 0; });

	uint64_t valueSum = 0;

	for (int i = 0; i < handBids.size(); i++) {
		valueSum += (i + 1) * handBids[i].bid;
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;


	fmt::print("Processed 1: {}\n", valueSum);
	//fmt::print("Processed 2: {}\n", minRangeLocation);


	fmt::print("Took {}\n", dur);

	return 0;
}
