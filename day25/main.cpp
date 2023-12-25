
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
#include <unordered_set>
#include <random>

#include <fmt/ranges.h>
#include <fmt/chrono.h>

using namespace std::string_view_literals;
using namespace std::string_literals;

std::vector<std::pair<std::string, std::string>> parseLine(std::string_view sv) {
	std::string leftStr{ sv.substr(0, sv.find_first_of(':')) };

	sv.remove_prefix(sv.find_first_of(':') + 2);

	return sv | std::views::split(" "sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); })
		| std::views::transform([&](auto sv) {
			return std::make_pair(leftStr, std::string{ sv });
		}) | std::ranges::to<std::vector>();
}

std::pair<std::string, std::string> sortInPair(std::pair<std::string, std::string> input) {

	if (input.first < input.second) {
		return input;
	} else {
		std::swap(input.first, input.second);
		return input;
	}
}

struct FromToRemoveInfo {
	std::string_view fromA;
	std::string_view toA;

	std::string_view fromB;
	std::string_view toB;

	std::string_view fromC;
	std::string_view toC;
};

int64_t travelSet(const std::map<std::string_view, std::vector<std::string_view>>& map, std::string_view startValue, std::set<std::string_view>& removeFrom, const FromToRemoveInfo& info) {
	std::unordered_set<std::string_view> foundValues;

	std::queue<std::string_view> toProcess;
	toProcess.push(startValue);

	bool fromAFound = false;
	bool toAFound = false;
	bool fromBFound = false;
	bool toBFound = false;
	bool fromCFound = false;
	bool toCFound = false;

	while (!toProcess.empty()) {
		auto value = toProcess.front();
		toProcess.pop();

		if (value == info.fromA) {
			fromAFound = true;
		}
		if (value == info.toA) {
			toAFound = true;
		}
		if (fromAFound && toAFound) {
			return -1;
		}
		if (value == info.fromB) {
			fromBFound = true;
		}
		if (value == info.toB) {
			toBFound = true;
		}
		if (fromBFound && toBFound) {
			return -1;
		}
		if (value == info.fromC) {
			fromCFound = true;
		}
		if (value == info.toC) {
			toCFound = true;
		}
		if (fromCFound && toCFound) {
			return -1;
		}

		if (foundValues.contains(value)) {
			continue;
		}

		foundValues.insert(value);
		removeFrom.erase(value);

		for (auto& next : map.at(value)) {
			toProcess.push(next);
		}
	}

	return foundValues.size();
}

struct Job {
	using It = std::vector<std::pair<std::string, std::string>>::const_iterator;

	It a;
	It b;

	const std::vector<std::pair<std::string, std::string>>* map;
	const std::vector<std::pair<std::string, std::string>>* links;

	const std::set<std::string_view>* knownValues;
};

struct Path {
	std::string_view currentNode;

	std::vector<std::string_view> pathHistory;
};

std::vector<std::pair<std::string_view, std::string_view>> pathfind(const std::map<std::string_view, std::vector<std::string_view>>& map, const std::string& first, const std::string& second) {
	std::queue<Path> toVisitQueue;

	std::unordered_set<std::string_view> visitedNodes;

	toVisitQueue.push(Path{ first, {} });

	while (!toVisitQueue.empty()) {
		auto path = std::move(toVisitQueue.front());
		toVisitQueue.pop();

		if (visitedNodes.contains(path.currentNode)) {
			continue;
		}

		visitedNodes.insert(path.currentNode);
		path.pathHistory.push_back(path.currentNode);

		if (path.currentNode == second) {
			std::vector<std::pair<std::string_view, std::string_view>> returnVec;
			for (int i = 1; i < path.pathHistory.size(); i++) {
				returnVec.emplace_back(path.pathHistory[i - 1], path.pathHistory[i]);
			}

			return returnVec;
		}

		for (auto& next : map.at(path.currentNode)) {
			auto newPath = path;
			newPath.currentNode = next;

			toVisitQueue.push(newPath);
		}
	}

	return {};
}

void normAndSortPath(std::vector<std::pair<std::string_view, std::string_view>>& pairs) {
	for (auto& elem : pairs) {
		if (elem.first > elem.second) {
			std::swap(elem.first, elem.second);
		}
	}

	std::ranges::sort(pairs);
}

std::optional<int64_t> processJob(Job& job, std::map<std::string_view, std::vector<std::string_view>>& fromToMap) {
	
	erase(fromToMap[job.a->first], job.a->second);
	erase(fromToMap[job.a->second], job.a->first);
	erase(fromToMap[job.b->first], job.b->second);
	erase(fromToMap[job.b->second], job.b->first);

	// now pathfind from aFrom, aTo and bFrom, bTo
	// get the paths, and find the common links (we could be going in either direction, so handle that)
	// if there are no common links, then there is no third link we can cut to split the cluster in two, so we return without a value
	// otherwise we only iterate the links

	auto pathA = pathfind(fromToMap, job.a->first, job.a->second);
	auto pathB = pathfind(fromToMap, job.b->first, job.a->second);

	normAndSortPath(pathA);
	normAndSortPath(pathB);

	std::vector<std::pair<std::string_view, std::string_view>> commonLinks;

	std::ranges::set_intersection(pathA, pathB, std::back_inserter(commonLinks));

	for (auto link : commonLinks) {
		erase(fromToMap[link.first], link.second);
		erase(fromToMap[link.second], link.first);


		std::set remainingValues = *job.knownValues;
		std::vector<int64_t> connectionSets;

		FromToRemoveInfo info{
			job.a->first, job.a->second,
			job.b->first, job.b->second,
			link.first, link.second
		};

		while (!remainingValues.empty()) {
			auto startValue = *remainingValues.begin();

			auto foundValues = travelSet(fromToMap, startValue, remainingValues, info);
			if (foundValues == -1) {
				// this map setup doesn't work
				connectionSets.clear();
				remainingValues.clear();
			}

			connectionSets.push_back(foundValues);

			// another bad run...
			if (connectionSets.size() == 2 && !remainingValues.empty()) {
				connectionSets.clear();
				remainingValues.clear();
			}
		}

		if (connectionSets.size() == 2) {
			return connectionSets[0] * connectionSets[1];
		}

		fromToMap[link.first].push_back(link.second);
		fromToMap[link.second].push_back(link.first);
	}

	fromToMap[job.b->first].push_back(job.b->second);
	fromToMap[job.b->second].push_back(job.b->first);
	fromToMap[job.a->first].push_back(job.a->second);
	fromToMap[job.a->second].push_back(job.a->first);
}

std::atomic_bool jobsRunning{true};
std::vector<Job> jobs;
std::mutex jobsMutex;

std::atomic_int numJobsProcessed{ 0 };
std::atomic_int64_t globalResult{ -1 };

std::map<std::string_view, std::vector<std::string_view>> generateFromToMap(const Job& job) {
	std::map<std::string_view, std::vector<std::string_view>> fromToMap;

	for (auto val = job.links->begin(); val != job.links->end(); ++val) {
		//if (val == a || val == b || val == c) {
		//	continue;
		//}

		fromToMap[val->first].push_back(val->second);
		fromToMap[val->second].push_back(val->first);
	}

	return fromToMap;
}

void runJobs() {

	std::vector<Job> localJobs;
	localJobs.reserve(10);

	std::map<std::string_view, std::vector<std::string_view>> fromToMap;
	bool fromToMapGenerated = false;

	while (jobsRunning) {
		

		// grab 10 jobs to run

		{
			std::scoped_lock lock{ jobsMutex };
			auto itLen = std::min(10ull, jobs.size());
			for (int i = 0; i < itLen; i++) {
				localJobs.push_back(std::move(jobs.back()));
				jobs.pop_back();
			}
		}

		while (!localJobs.empty()) {
			auto& job = localJobs.back();

			if (!fromToMapGenerated) {
				fromToMap = generateFromToMap(job);
				fromToMapGenerated = true;
			}

			auto result = processJob(job, fromToMap);
			++numJobsProcessed;

			if (result) {
				globalResult = *result;
				jobsRunning = false;
				localJobs.clear();
			} else {
				localJobs.pop_back();
			}
		}
	}
}

int64_t find2waySplitSum(const std::vector<std::pair<std::string, std::string>>& links, const std::set<std::string_view>& knownValues) {

	jobs.reserve(links.size() * links.size());

	for (auto a = links.begin(); a != links.end(); ++a) {
		for (auto b = a + 1; b != links.end(); ++b) {
			jobs.push_back(Job{
				a, b, &links, &links, &knownValues
			});
		}
	}
	std::random_device rd;

	std::shuffle(jobs.begin(), jobs.end(), rd);

	int64_t numJobs = jobs.size();

	std::vector<std::jthread> threads;

	for (int i = 0; i < std::thread::hardware_concurrency(); i++) {
		threads.emplace_back(runJobs);
	}

	while (jobsRunning) {
		auto count = numJobsProcessed.load(std::memory_order_release);
		auto percent = static_cast<float>(count) / numJobs * 100.0f;

		fmt::print("{}/{} - {}%\n", count, numJobs, percent);

		std::this_thread::sleep_for(std::chrono::seconds{ 1 });
	}

	return globalResult;
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day25.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	//auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	//auto nonEmptyLines = lines | std::views::filter([](auto sv) { return !sv.empty(); });

	auto lines = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); }) | std::views::filter([](auto sv) { return !sv.empty(); });

	auto connections = lines | std::views::transform(parseLine) | std::ranges::views::join | std::ranges::to<std::vector>();

	auto connectionsValueSorted = connections | std::views::transform(sortInPair) | std::ranges::to<std::vector>();
	std::ranges::sort(connectionsValueSorted);
	const auto [removeFirst, removeLast] = std::ranges::unique(connectionsValueSorted);
	connectionsValueSorted.erase(removeFirst, removeLast);

	std::set<std::string_view> knownPoints;
	for (auto& [from, to] : connectionsValueSorted) {
		knownPoints.insert(from);
		knownPoints.insert(to);
	}

	auto splitSum = find2waySplitSum(connectionsValueSorted, knownPoints);

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	//debugPrintMapPath(map, *longestPath);


	fmt::print("Processed 1: {}\n", splitSum);
	//fmt::print("Processed 2: {}\n", longestPath2->currentSteps);




	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
