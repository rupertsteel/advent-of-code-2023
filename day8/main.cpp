
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

struct Node {
	std::string leftNode;
	std::string rightNode;
};

std::pair<std::string, Node> parseNode(std::string_view sv) {
	std::string nodeName{ sv.substr(0, 3) };

	std::string leftNodeName{ sv.substr(7, 3) };
	std::string rightNodeName{ sv.substr(12, 3) };

	return std::make_pair(nodeName, Node{ leftNodeName, rightNodeName });
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day8.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	auto nonEmptyLines = lines | std::views::filter([](auto sv) { return !sv.empty(); });

	auto instructionLine = *nonEmptyLines.begin();

	auto nodes = nonEmptyLines | std::views::drop(1) | std::views::transform(parseNode) | std::ranges::to<std::map>();

	std::string currentNode = "AAA";

	uint64_t currentStep = 0;

	while (currentNode != "ZZZ") {
		if (instructionLine[currentStep % instructionLine.size()] == 'L') {
			currentNode = nodes[currentNode].leftNode;
		} else {
			currentNode = nodes[currentNode].rightNode;
		}

		++currentStep;
	}


	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;


	fmt::print("Processed 1: {}\n", currentStep);
	//fmt::print("Processed 2: {}\n", minRangeLocation);


	fmt::print("Took {}\n", dur);

	return 0;
}
