
#include "catch2/catch_test_macros.hpp"

#include "core.hpp"

TEST_CASE("Test TrueBitRange", "[core]") {
	core::TrueBitRange rng1{ 0 };

	CHECK(rng1.size() == 0);
	auto rng1It = rng1.begin();
	CHECK(rng1It == rng1.end());

	core::TrueBitRange rng2{ 0b0010'0110ull };
	REQUIRE(rng2.size() == 3);
	auto rng2It = rng2.begin();
	REQUIRE(rng2It != rng2.end());
	CHECK(*rng2It == 1);
	++rng2It;
	REQUIRE(rng2It != rng2.end());
	CHECK(*rng2It == 2);
	++rng2It;
	REQUIRE(rng2It != rng2.end());
	CHECK(*rng2It == 5);
	++rng2It;
	CHECK(rng2It == rng2.end());

	core::TrueBitRange rng3{ std::numeric_limits<uint64_t>::max() };
	REQUIRE(rng3.size() == 64);
	auto rng3It = rng3.begin();
	for (int i = 0; i < 64; i++) {
		REQUIRE(rng3It != rng3.end());
		CHECK(*rng3It == i);
		++rng3It;
	}
}
