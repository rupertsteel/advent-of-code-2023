
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

struct GameWorking {
	int gameId;
	std::vector<std::valarray<int>> rounds;
};

struct Point {
	int x;
	int y;

	std::strong_ordering operator<=>(const Point&) const = default;

	Point operator+(const Point& other) const {
		return { x + other.x, y + other.y };
	}
};

struct Word {
	std::string digits;
};

std::array<Point, 8> locationOffsets = { {
	{ 1, 0 },
	{ 1, 1},
	{ 0, 1},
	{ -1, 1},
	{ -1, 0},
	{-1, -1},
	{ 0, -1},
	{1, -1},
} };

int main(int argc, char* argv[]) {


	std::ifstream inputFile("inputs/day3.txt");
	std::string input(std::istreambuf_iterator{ inputFile }, std::istreambuf_iterator<char>{});

	auto start = std::chrono::high_resolution_clock::now();

	auto processed1 = std::string_view{ input } | std::views::split("\n"sv) | std::views::transform([](auto rng) { return std::string_view(rng.begin(), rng.end()); });
	auto processed2 = processed1 | std::views::filter([](auto sv) { return !sv.empty(); });

	std::vector<std::unique_ptr<Word>> numbers;
	std::vector<std::unique_ptr<Word>> symbols;
	std::map<Point, Word*> positionToNumber;
	std::map<Point, Word*> positionToSymbol;

	std::set<Word*> partNumberWords;

	int sum2 = 0;

	int row = 0;
	for (const auto& line : processed2) {

		int column = 0;
		while (column < line.length()) {
			if (line[column] == '.') {
				column++;
				continue;
			}

			if (ispunct(line[column])) {
				auto newWord = std::make_unique<Word>(std::string{ line[column] });

				positionToSymbol.insert(std::make_pair(Point{ column, row }, newWord.get()));

				symbols.push_back(std::move(newWord));
				column++;
				continue;
			}

			if (isdigit(line[column])) {
				int initialColumn = column;

				std::string word;
				word.push_back(line[column]);

				while ((column + 1) < line.length() && isdigit(line[column + 1])) {
					column++;
					word.push_back(line[column]);
				}

				column++;

				auto newWord = std::make_unique<Word>(word);

				for (; initialColumn < column; initialColumn++) {
					positionToNumber.insert(std::make_pair(Point{ initialColumn, row }, newWord.get()));
				}

				numbers.push_back(std::move(newWord));
			}
		}

		row++;
	}

	for (const auto& symbol : positionToSymbol) {

		std::set<Word*> symbolWords;

		for (int i = 0; i < 8; i++) {
			auto offsetPoint = symbol.first + locationOffsets[i];

			if (positionToNumber.contains(offsetPoint)) {
				partNumberWords.insert(positionToNumber.at(offsetPoint));
				symbolWords.insert(positionToNumber.at(offsetPoint));

			}
		}

		for (auto point : symbolWords) {
			auto number = std::stoi(point->digits);
			sum2 += number;
		}
	}

	int sum = 0;

	for (auto point : partNumberWords) {
		auto number = std::stoi(point->digits);
		sum += number;
	}


	auto end = std::chrono::high_resolution_clock::now();
	auto dur = end - start;


	fmt::print("Processed 1: {}\n", sum);
	fmt::print("Processed 2: {}\n", sum2);


	fmt::print("Took {}\n", dur);

	return 0;
}