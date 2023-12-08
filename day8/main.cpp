
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

std::vector<std::string> findGhostStartNodes(const std::map<std::string, Node>& map) {
	std::vector<std::string> returnVec;

	for (auto& [key, value] : map) {
		if (key[2] == 'A') {
			returnVec.push_back(key);
		}
	}

	return returnVec;
}

struct CycleInfo {
	uint64_t stableStartPoint;
	uint64_t cycleLength;
};

struct CycleWorking {
	std::string endNode;
	uint64_t step;
};

CycleInfo findCycleInfo(const std::map<std::string, Node>& map, std::string startNode, std::string_view instructionLine) {

	std::string currentNode = startNode;

	uint64_t currentStep = 0;

	std::deque<CycleWorking> working;

	while (true) {
		if (currentNode[2] == 'Z') {
			// this is a end node, break if we think we have found a loop

			if (working.size() == 10) {
				working.pop_front();
			}
			working.push_back({ currentNode, currentStep });

			fmt::println("Start Node: {}, Found End: {}, Step: {}", startNode, currentNode, currentStep);

			if (working.size() < 5) {
				fmt::println("Need more info, continuing...");
			} else {
				if (std::ranges::all_of(working, [&](const auto& node) { return node.endNode == working[0].endNode; })) {
					std::vector<uint64_t> deltas;
					for (int i = 1; i < working.size(); i++) {
						deltas.push_back(working[i].step - working[i - 1].step);
					}

					if (std::ranges::all_of(deltas, [&](const auto val) { return val == deltas[0];})) {
						// deltas of all nodes is the same, base our cycle from the first node
						return { working[0].step, deltas[0] };
					}
				}
			}
		}

		if (instructionLine[currentStep % instructionLine.size()] == 'L') {
			currentNode = map.at(currentNode).leftNode;
		} else {
			currentNode = map.at(currentNode).rightNode;
		}

		++currentStep;
	}
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

	std::vector<std::string> ghostCurrentNodes = findGhostStartNodes(nodes);

	auto cycles = ghostCurrentNodes | std::views::transform([&](const std::string& str) {
		return findCycleInfo(nodes, str, instructionLine);
	}) | std::ranges::to<std::vector>();

	// the cycles seem to be stable, so the cycle start length isn't needed
	uint64_t ghostEndStep = cycles[0].cycleLength;

	for (auto& val : cycles) {
		ghostEndStep = std::lcm(ghostEndStep, val.cycleLength);
	}


	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;


	fmt::print("Processed 1: {}\n", currentStep);
	fmt::print("Processed 2: {}\n", ghostEndStep);


	fmt::print("Took {}\n", dur);

	return 0;
}
