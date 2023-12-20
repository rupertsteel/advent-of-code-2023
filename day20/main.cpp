
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

struct Message {
	std::string src;
	std::string dest;
	bool high;
};

struct FlipFlopModule {
	std::string self;
	std::vector<std::string> destinations;
	bool on = false;

	void process(const Message& message, std::queue<Message>& messages) {
		if (message.high) {
			return;
		}

		on = !on;

		for (auto& dest : destinations) {
			messages.push(Message{ self, dest, on });
		}
	}
};

struct ConjunctionModule {
	std::string self;
	std::vector<std::string> destinations;
	std::map<std::string, bool, std::less<>> inputMemory;

	void primeInput(const std::string& name) {
		inputMemory[name] = false;
	}

	void process(const Message& message, std::queue<Message>& messages) {
		inputMemory[message.src] = message.high;

		if (std::ranges::all_of(inputMemory | std::views::values, [](auto val) { return val; })) {
			for (auto& dest : destinations) {
				messages.push(Message{ self, dest, false });
			}
		} else {
			for (auto& dest : destinations) {
				messages.push(Message{ self, dest, true });
			}
		}
	}
};

struct System {
	std::vector<std::string> broadcasterSendTo;
	std::map<std::string, FlipFlopModule> flipFlopModules;
	std::map<std::string, ConjunctionModule> conjunctionModules;
};

void parseBroadcaster(System& system, std::string_view line) {
	line.remove_prefix(15);

	while (line.find_first_of(',') != std::string_view::npos) {
		auto commaEnd = line.find_first_of(',');

		auto dest = line.substr(0, commaEnd);

		line.remove_prefix(commaEnd + 2);

		system.broadcasterSendTo.emplace_back(dest);
	}

	system.broadcasterSendTo.emplace_back(line);
}

void parseFlipFlop(System& system, std::string_view line) {
	line.remove_prefix(1);

	auto nameEnd = line.find_first_of(' ');
	std::string name{ line.substr(0, nameEnd) };

	line.remove_prefix(nameEnd + 4);

	std::vector<std::string> dests;
	while (line.find_first_of(',') != std::string_view::npos) {
		auto commaEnd = line.find_first_of(',');

		auto dest = line.substr(0, commaEnd);

		line.remove_prefix(commaEnd + 2);

		dests.emplace_back(dest);
	}

	dests.emplace_back(line);

	system.flipFlopModules[name] = FlipFlopModule{ name, dests, false };
}

void parseConjunction(System& system, std::string_view line) {
	line.remove_prefix(1);

	auto nameEnd = line.find_first_of(' ');
	std::string name{ line.substr(0, nameEnd) };

	line.remove_prefix(nameEnd + 4);

	std::vector<std::string> dests;
	while (line.find_first_of(',') != std::string_view::npos) {
		auto commaEnd = line.find_first_of(',');

		auto dest = line.substr(0, commaEnd);

		line.remove_prefix(commaEnd + 2);

		dests.emplace_back(dest);
	}

	dests.emplace_back(line);

	system.conjunctionModules[name] = ConjunctionModule{ name, dests };
}

System parseSystem(std::string_view input) {
	auto lines = input | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); })
		| std::views::filter([](auto sv) { return !sv.empty(); });

	System tmpSystem;

	for (auto line : lines) {
		if (line.starts_with("broadcaster")) {
			parseBroadcaster(tmpSystem, line);
		}

		if (line.starts_with("%")) {
			parseFlipFlop(tmpSystem, line);
		}

		if (line.starts_with("&")) {
			parseConjunction(tmpSystem, line);
		}
	}

	for (auto& broadcastOutput : tmpSystem.broadcasterSendTo) {
		if (tmpSystem.conjunctionModules.contains(broadcastOutput)) {
			tmpSystem.conjunctionModules[broadcastOutput].primeInput("broadcaster");
		}
	}
	for (auto& flipFlopModule : tmpSystem.flipFlopModules) {
		for (auto& dest : flipFlopModule.second.destinations) {
			if (tmpSystem.conjunctionModules.contains(dest)) {
				tmpSystem.conjunctionModules[dest].primeInput(flipFlopModule.first);
			}
		}
	}
	for (auto& conjunctionModule : tmpSystem.conjunctionModules) {
		for (auto& dest : conjunctionModule.second.destinations) {
			if (tmpSystem.conjunctionModules.contains(dest)) {
				tmpSystem.conjunctionModules[dest].primeInput(conjunctionModule.first);
			}
		}
	}

	return tmpSystem;
}

uint64_t countSignals(System system, int inputTimes) {
	uint64_t lowSignals = 0;
	uint64_t highSignals = 0;

	for (int i = 0; i < inputTimes; i++) {
		std::queue<Message> messageQueue;

		messageQueue.push(Message{ "button", "broadcaster", false });

		while (!messageQueue.empty()) {
			auto signal = messageQueue.front();
			messageQueue.pop();

			if (signal.high) {
				highSignals++;
			} else {
				lowSignals++;
			}

			if (signal.dest == "broadcaster") {
				for (auto& dest : system.broadcasterSendTo) {
					messageQueue.push(Message{ "broadcaster", dest, signal.high });
				}
			} else {
				if (system.flipFlopModules.contains(signal.dest)) {
					system.flipFlopModules[signal.dest].process(signal, messageQueue);
				}
				if (system.conjunctionModules.contains(signal.dest)) {
					system.conjunctionModules[signal.dest].process(signal, messageQueue);
				}
			}
		}
	}

	return lowSignals * highSignals;
}

int main(int argc, char* argv[]) {
	std::ifstream inputFile("inputs/day20.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	auto system = parseSystem(input);

	auto part1Signals = countSignals(system, 1000);

	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;

	fmt::print("Processed 1: {}\n", part1Signals);
	//fmt::print("Processed 2: {}\n", totalAcceptValues);


	fmt::print("Took {}\n", std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(dur));

	return 0;
}
