/**
 * TestSmallCover.cpp
 *
 * By Sebastian Raaphorst, 2018.
 */

#include <array>

#include <catch.hpp>
#include <dlx_contexpr.h>

TEST_CASE("Small exact cover") {
    constexpr dlx::position_array<10> positions {
            /** r0 **/ std::make_pair(0, 0), std::make_pair(0, 2), std::make_pair(0, 4),
            /** r1 **/ std::make_pair(1, 0), std::make_pair(1, 1), std::make_pair(1, 3), std::make_pair(1, 5),
            /** r2 **/ std::make_pair(2, 1), std::make_pair(2, 3),
            /** r3 **/ std::make_pair(3, 5)
    };
    constexpr auto solution = dlx::DLX<6, 4, 10>::run(positions);

    REQUIRE(solution.has_value());

    constexpr std::array<bool, 4> expected{{true, false, true, true}};
    REQUIRE(solution == expected);
}