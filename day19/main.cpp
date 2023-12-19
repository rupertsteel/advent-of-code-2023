
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
#include <unordered_map>

#include <fmt/ranges.h>
#include <fmt/chrono.h>

using namespace std::string_view_literals;
using namespace std::string_literals;

struct Point {
	int64_t x;
	int64_t y;

	std::strong_ordering operator<=>(const Point&) const = default;
	bool operator==(const Point&) const = default;


	Point operator+(const Point& other) const {
		return Point{ x + other.x, y + other.y };
	}
};

struct Block {
	std::optional<uint32_t> color;
	bool dug;
	bool outside;
};

struct Map {
	Point topLeftInclusive;
	Point bottomRightInclusive;
	std::map<Point, Block> map;

	void debugPrint() {
		std::string printStr;

		for (int y = topLeftInclusive.y; y <= bottomRightInclusive.y; y++) {
			for (int x = topLeftInclusive.x; x <= bottomRightInclusive.x; x++) {
				if (map.contains(Point{ x, y }) && map[Point{x, y}].dug) {
					printStr += '#';
				} else {
					printStr += '.';
				}
			}

			printStr += '\n';
		}

		fmt::print("{}\n", printStr);
	}
};

struct Instruction {
	Point directionDiff;
	int amount;
};

struct Item {
	int x;
	int m;
	int a;
	int s;

	std::vector<std::string> rulesThrough;
};

struct Range {
	int minInclusive;
	int maxInclusive;
};

struct RangeItem {
	Range x;
	Range m;
	Range a;
	Range s;
};

struct Rule {
	int Item::* checkVar;
	Range RangeItem::* rangeCheckVar;
	bool checkGreater;
	int compareValue;

	std::string destQueue;
};

struct Workflow {
	std::string name;

	std::vector<Rule> rules;

	std::string fallthroughQueue;
};

Workflow parseWorkflow(std::string_view sv) {
	auto bracePos = sv.find_first_of('{');

	auto strValue = sv.substr(0, bracePos);

	sv.remove_prefix(bracePos + 1);

	Workflow tmpWorkflow;
	tmpWorkflow.name = strValue;

	while (sv.find_first_of(',') != std::string_view::npos) {
		Rule tmpRule;

		switch (sv[0]) {
		case 'x':
			tmpRule.checkVar = &Item::x;
			tmpRule.rangeCheckVar = &RangeItem::x;
			break;
		case 'm':
			tmpRule.checkVar = &Item::m;
			tmpRule.rangeCheckVar = &RangeItem::m;
			break;
		case 'a':
			tmpRule.checkVar = &Item::a;
			tmpRule.rangeCheckVar = &RangeItem::a;
			break;
		case 's':
			tmpRule.checkVar = &Item::s;
			tmpRule.rangeCheckVar = &RangeItem::s;
			break;
		default:
			throw std::runtime_error("");
		}

		if (sv[1] == '>') {
			tmpRule.checkGreater = true;
		} else {
			tmpRule.checkGreater = false;
		}

		sv.remove_prefix(2);

		auto numEnd = sv.find_first_of(':');
		tmpRule.compareValue = std::stoi(std::string(sv.substr(0, numEnd)));

		sv.remove_prefix(numEnd + 1);

		auto strEnd = sv.find_first_of(',');
		tmpRule.destQueue = sv.substr(0, strEnd);
		sv.remove_prefix(strEnd + 1);

		tmpWorkflow.rules.push_back(tmpRule);
	}

	auto endBrace = sv.find_first_of('}');
	tmpWorkflow.fallthroughQueue = sv.substr(0, endBrace);

	return tmpWorkflow;
}

Item parseValue(std::string_view sv) {
	Item tmpItem;

	sv.remove_prefix(3);
	auto numEnd = sv.find_first_of(',');
	tmpItem.x = std::stoi(std::string(sv.substr(0, numEnd)));
	sv.remove_prefix(numEnd + 1);

	sv.remove_prefix(2);
	numEnd = sv.find_first_of(',');
	tmpItem.m = std::stoi(std::string(sv.substr(0, numEnd)));
	sv.remove_prefix(numEnd + 1);

	sv.remove_prefix(2);
	numEnd = sv.find_first_of(',');
	tmpItem.a = std::stoi(std::string(sv.substr(0, numEnd)));
	sv.remove_prefix(numEnd + 1);

	sv.remove_prefix(2);
	numEnd = sv.find_first_of('}');
	tmpItem.s = std::stoi(std::string(sv.substr(0, numEnd)));
	sv.remove_prefix(numEnd + 1);

	return tmpItem;
}

uint64_t processItems(std::vector<Workflow> workflows, std::vector<Item> values) {
	std::map<std::string, std::deque<Item>> queueMaps;
	std::map<std::string, Workflow> workflowMap;
	std::set<std::string> queuesToProcess;

	for (auto& workflow : workflows) {
		workflowMap[workflow.name] = workflow;
		queueMaps[workflow.name];
	}
	for (auto& value : values) {
		queueMaps["in"].push_back(value);
	}
	queuesToProcess.insert("in");

	while (!queuesToProcess.empty()) {
		auto nextQueueToProcess = *queuesToProcess.begin();

		queuesToProcess.erase(queuesToProcess.begin());

		if (nextQueueToProcess == "A" || nextQueueToProcess == "R") {
			continue;
		}

		auto& queue = queueMaps[nextQueueToProcess];
		auto& workflow = workflowMap[nextQueueToProcess];

		while (!queue.empty()) {
			auto item = queue.front();
			queue.pop_front();

			item.rulesThrough.push_back(workflow.name);

			bool processed = false;

			for (auto& rule : workflow.rules) {
				bool ruleAccept;

				if (rule.checkGreater) {
					ruleAccept = item.*(rule.checkVar) > rule.compareValue;
				} else {
					ruleAccept = item.*(rule.checkVar) < rule.compareValue;
				}

				if (ruleAccept) {
					queueMaps[rule.destQueue].push_back(item);
					queuesToProcess.insert(rule.destQueue);
					processed = true;
					break;
				}
			}

			if (!processed) {
				queueMaps[workflow.fallthroughQueue].push_back(item);
				queuesToProcess.insert(workflow.fallthroughQueue);
			}
		}
	}

	uint64_t scoreSum = 0;
	auto& acceptQueue = queueMaps["A"];

	for (auto& item : acceptQueue) {
		scoreSum += (item.a + item.m + item.s + item.x);
	}

	return scoreSum;
}

uint64_t processWorkflowRanges(const std::vector<Workflow>& workflows) {
	std::map<std::string, std::deque<RangeItem>> queueMaps;
	std::map<std::string, Workflow> workflowMap;
	std::set<std::string> queuesToProcess;

	for (auto& workflow : workflows) {
		workflowMap[workflow.name] = workflow;
		queueMaps[workflow.name];
	}
	queueMaps["in"].push_front(RangeItem{ {1, 4000}, {1, 4000}, {1, 4000}, {1, 4000} });
	queuesToProcess.insert("in");

	while (!queuesToProcess.empty()) {
		auto nextQueueToProcess = *queuesToProcess.begin();

		queuesToProcess.erase(queuesToProcess.begin());

		if (nextQueueToProcess == "A" || nextQueueToProcess == "R") {
			continue;
		}

		auto& queue = queueMaps[nextQueueToProcess];
		auto& workflow = workflowMap[nextQueueToProcess];

		while (!queue.empty()) {
			auto item = queue.front();
			queue.pop_front();

			//item.rulesThrough.push_back(workflow.name);

			bool processed = false;

			for (auto& rule : workflow.rules) {
				// split item into two items

				auto passItem = item;
				auto failItem = item;

				// split the range
				if (rule.checkGreater) {
					(passItem.*(rule.rangeCheckVar)).minInclusive = std::max((passItem.*(rule.rangeCheckVar)).minInclusive, rule.compareValue + 1);
					(failItem.*(rule.rangeCheckVar)).maxInclusive = std::min((failItem.*(rule.rangeCheckVar)).maxInclusive, rule.compareValue);
				} else {
					(passItem.*(rule.rangeCheckVar)).maxInclusive = std::min((passItem.*(rule.rangeCheckVar)).maxInclusive, rule.compareValue - 1);
					(failItem.*(rule.rangeCheckVar)).minInclusive = std::max((failItem.*(rule.rangeCheckVar)).minInclusive, rule.compareValue);
				}

				// send the pass item if it makes sense
				if ((passItem.*rule.rangeCheckVar).minInclusive <= (passItem.*rule.rangeCheckVar).maxInclusive) {
					queueMaps[rule.destQueue].push_back(passItem);
					queuesToProcess.insert(rule.destQueue);
				}

				item = failItem;

				// if the item no longer makes sense, we need to skip
				if ((failItem.*rule.rangeCheckVar).minInclusive > (failItem.*rule.rangeCheckVar).maxInclusive) {
					processed = true;
					break;
				}
			}

			if (!processed) {
				queueMaps[workflow.fallthroughQueue].push_back(item);
				queuesToProcess.insert(workflow.fallthroughQueue);
			}
		}
	}

	uint64_t scoreSum = 0;
	auto& acceptQueue = queueMaps["A"];

	for (auto& item : acceptQueue) {
		scoreSum += static_cast<uint64_t>(item.a.maxInclusive - item.a.minInclusive + 1) * (item.m.maxInclusive - item.m.minInclusive + 1) * (item.s.maxInclusive - item.s.minInclusive + 1) * (item.x.maxInclusive - item.x.minInclusive + 1);
	}

	return scoreSum;
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day19.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	auto blocks = std::string_view{ input } | std::views::split("\n\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); }) | std::ranges::to<std::vector>();

	auto workflows = blocks[0] | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); })
		| std::views::filter([](auto sv) { return !sv.empty(); }) | std::views::transform(parseWorkflow) | std::ranges::to<std::vector>();

	auto values = blocks[1] | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); })
		| std::views::filter([](auto sv) { return !sv.empty(); }) | std::views::transform(parseValue) | std::ranges::to<std::vector>();


	auto scoreSum = processItems(workflows, values);
	auto totalAcceptValues = processWorkflowRanges(workflows);

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	fmt::print("Processed 1: {}\n", scoreSum);
	fmt::print("Processed 2: {}\n", totalAcceptValues);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
