
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
#include <ratio>
#include <execution>
#include <semaphore>
#include <stack>
#include <unordered_map>

#include <fmt/ranges.h>
#include <fmt/chrono.h>

using namespace std::string_view_literals;
using namespace std::string_literals;

struct Point {
	int x;
	int y;

	std::strong_ordering operator<=>(const Point&) const = default;
	bool operator==(const Point&) const = default;


	Point operator+(const Point& other) const {
		return Point{ x + other.x, y + other.y };
	}
};

struct Block {
	int width;
	int height;
	std::map<Point, char> map;

	void debugPrint() {
		std::string printStr;

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				if (map.contains(Point{ x, y })) {
					printStr += map[Point{ x, y }];
				} else {
					printStr += '.';
				}
			}

			printStr += '\n';
		}

		fmt::print("{}\n", printStr);
	}
};

std::array<Point, 4> plusOffsets{ {
		{ -1, 0},
	{ 1, 0},
	{ 0, 1},
	{ 0, -1}
} };

struct Path {
	int currentNode;

	int currentSteps;

	//std::set<int> exploredNodes;
	//std::flat_set<int> exploredNodes;
	std::vector<int> exploredNodes;
};

struct NodeLink {
	int cost;
	int nextNodeId;
};

struct NodeElem {
	int id;
	bool isStart;
	bool isEnd;

	std::vector<NodeLink> nextNodes;
};

struct NodeMap {
	std::map<int, NodeElem> nodes;

	int startNodeId;
	int endNodeId;
};

NodeMap convertToNodeMap(const Block& inputMap) {
	std::map<Point, int> positionToNodeIds;

	NodeMap map;

	int nextNodeId = 0;

	for (int y = 0; y < inputMap.height; y++) {
		for (int x = 0; x < inputMap.width; x++) {
			auto ch = inputMap.map.at(Point{ x, y });

			if (ch == '#') {
				continue;
			}

			if (y == 0) {
				map.startNodeId = nextNodeId;
			}
			if (y == inputMap.height - 1) {
				map.endNodeId = nextNodeId;
			}
			positionToNodeIds[Point{ x, y }] = nextNodeId++;
			
		}
	}

	for (auto& [position, nodeId] : positionToNodeIds) {

		NodeElem node;
		node.id = nodeId;

		node.isStart = position.y == 0;
		node.isEnd = position.y == inputMap.height - 1;

		// check neighbors
		for (int i = 0; i < 4; i++) {
			auto checkPosition = position + plusOffsets[i];
			if (checkPosition.x < 0 || checkPosition.y < 0 || checkPosition.x >= inputMap.width || checkPosition.y >= inputMap.height) {
				continue;
			}

			if (!positionToNodeIds.contains(checkPosition)) {
				continue;
			}

			auto nextId = positionToNodeIds.at(checkPosition);
			node.nextNodes.push_back(NodeLink{ 1, nextId });
		}

		map.nodes[node.id] = node;
	}

	return map;
}

NodeMap simplifyNodeMap(NodeMap nodeMap) {

	while (true) {
		bool simplifiedThisRound = false;

		for (auto it = nodeMap.nodes.begin(); it != nodeMap.nodes.end();) {
			if (it->second.nextNodes.size() == 2) {
				// we can simplify this, move the weights to either sides
				auto prevNode = it->second.nextNodes[0];
				auto nextNode = it->second.nextNodes[1];

				for (auto& elem : nodeMap.nodes[prevNode.nextNodeId].nextNodes) {
					if (elem.nextNodeId == it->first) {
						elem.nextNodeId = nextNode.nextNodeId;
						elem.cost += nextNode.cost;
					}
				}
				for (auto& elem : nodeMap.nodes[nextNode.nextNodeId].nextNodes) {
					if (elem.nextNodeId == it->first) {
						elem.nextNodeId = prevNode.nextNodeId;
						elem.cost += prevNode.cost;
					}
				}

				it = nodeMap.nodes.erase(it);
				simplifiedThisRound = true;
			} else {
				++it;
			}
		}

		if (!simplifiedThisRound) {
			break;
		}
	}

	return nodeMap;
}


std::vector<Path> pathSearch(const NodeMap& map) {
	Path startingPath;
	startingPath.currentNode = map.startNodeId;
	startingPath.currentSteps = 0;
	//startingPath.exploredNodes.insert(map.startNodeId);
	startingPath.exploredNodes.push_back(map.startNodeId);

	std::stack<Path> pathsToSearch;
	pathsToSearch.push(startingPath);

	std::vector<Path> endPaths;

	uint64_t it = 0;

	auto lastPrint = std::chrono::high_resolution_clock::now();

	while (!pathsToSearch.empty()) {
		++it;
		if (it % 100'000 == 0) {
			auto printTime = std::chrono::high_resolution_clock::now();
			auto dur = printTime - lastPrint;
			auto rate = 100000.0f / std::chrono::duration_cast<std::chrono::duration<float>>(dur).count();

			lastPrint = printTime;

			fmt::println("Processing it {}, depth {}, {} it/s", it, pathsToSearch.size(), rate);
		}

		// grab the path
		auto processPath = pathsToSearch.top();
		pathsToSearch.pop();

		if (processPath.currentNode == map.endNodeId) {
			endPaths.push_back(processPath);
			continue;
		}

		for (int i = 0; i < map.nodes.at(processPath.currentNode).nextNodes.size(); i++) {
			auto nextNode = map.nodes.at(processPath.currentNode).nextNodes[i];

			//if (processPath.exploredNodes.contains(nextNode.nextNodeId)) {
			if (std::ranges::contains(processPath.exploredNodes, nextNode.nextNodeId)) {
				continue;
			}

			auto newPath = processPath;
			newPath.currentNode = nextNode.nextNodeId;
			newPath.currentSteps += nextNode.cost;
			//newPath.exploredNodes.insert(nextNode.nextNodeId);
			newPath.exploredNodes.push_back(nextNode.nextNodeId);

			pathsToSearch.push(newPath);
		}
	}

	return endPaths;
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day23.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	//auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	//auto nonEmptyLines = lines | std::views::filter([](auto sv) { return !sv.empty(); });

	auto blocks = std::string_view{ input } | std::views::split("\n\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });

	auto maps = blocks | std::views::transform([](std::string_view sv) {
		Block tmpBlock;

		auto rowStr = std::views::zip(std::views::iota(0), sv | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); })
			| std::views::filter([](auto sv) { return !sv.empty(); }));

		for (auto val : rowStr) {
			auto row = std::get<0>(val);

			auto values = std::get<1>(val);

			for (int i = 0; i < values.size(); i++) {
				tmpBlock.map[Point{ i, row }] = values[i];
			}
			tmpBlock.width = values.size();
			tmpBlock.height = row + 1;
		}

		return tmpBlock;
	}) | std::ranges::to<std::vector>();

	auto map = maps[0];

	auto nodeMap = convertToNodeMap(map);

	auto simplifiedMap = simplifyNodeMap(nodeMap);

	/*auto paths = pathSearch(map, true);
	auto longestPath = std::ranges::max_element(paths, {}, [](auto& elem) {
		return elem.currentSteps;
		});*/

	auto paths2 = pathSearch(simplifiedMap);
	auto longestPath2 = std::ranges::max_element(paths2, {}, [](auto& elem) {
		return elem.currentSteps;
		});

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	//debugPrintMapPath(map, *longestPath);

	//fmt::print("Processed 1: {}\n", longestPath->currentSteps);
	fmt::print("Processed 2: {}\n", longestPath2->currentSteps);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
