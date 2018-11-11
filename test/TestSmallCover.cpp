/**
 * TestSmallCover.cpp
 *
 * By Sebastian Raaphorst, 2018.
 */

#include <array>

#include <catch.hpp>
#include <dlx_contexpr.h>

TEST_CASE("Small exact cover 1") {
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

TEST_CASE("Small exact cover 2") {
    // Over 10 columns
    //   0 1 2 3 4 5 6 7 8 9
    // 0 1 1 0 0 0 0 0 0 0 0
    // 1 0 1 1 0 0 0 0 0 0 0
    // 2 0 0 1 1 0 0 0 0 0 0
    // 3 0 0 0 1 1 0 0 0 0 0
    // 4 0 0 0 0 1 1 0 0 0 0
    // 5 0 0 0 0 0 1 1 0 0 0
    // 6 0 0 0 0 0 0 1 1 0 0
    // 7 0 0 0 0 0 0 0 1 1 0
    // 8 0 0 0 0 0 0 0 0 1 1
    constexpr dlx::position_array<18> positions {{
        {0, 0}, {0, 1},
        {1, 1}, {1, 2},
        {2, 2}, {2, 3},
        {3, 3}, {3, 4},
        {4, 4}, {4, 5},
        {5, 5}, {5, 6},
        {6, 6}, {6, 7},
        {7, 7}, {7, 8},
        {8, 8}, {8, 9},
    }};

    auto solution = dlx::DLX<10, 9, 18>::run(positions);
    REQUIRE(solution.has_value());
}

TEST_CASE("Small exact cover 3") {
    // Over 10 columns
    //   0 1 2 3 4 5 6 7 8 9
    // 0 1 1 0 0 0 0 0 0 0 0
    // 1 0 1 1 0 0 0 0 0 0 0
    // 2 0 0 1 1 0 0 0 0 0 0
    // 3 0 0 0 1 1 0 0 0 0 0
    // 4 0 0 0 0 1 1 0 0 0 0
    // 5 0 0 0 0 0 1 1 0 0 0
    // 6 0 0 0 0 0 0 1 1 0 0
    // 7 0 0 0 0 0 0 0 1 1 0
    // 8 0 0 0 0 0 0 0 0 1 1
    // 9 0 0 0 0 0 0 0 1 0 1
    // a 0 0 0 0 0 0 1 0 1 0
    // b 0 0 0 0 0 1 0 1 0 0
    // c 0 0 0 0 1 0 1 0 0 0
    // d 0 0 0 1 0 1 0 0 0 0
    // e 0 0 1 0 1 0 0 0 0 0
    // f 0 1 0 1 0 0 0 0 0 0
    // g 1 0 1 0 0 0 0 0 0 0
    // h 0 0 0 0 0 0 0 0 0 1
    constexpr dlx::position_array<23> positions {{
        {0, 0}, {0, 1},
        {1, 1}, {1, 2},
        {2, 2}, {2, 3},
        {3, 3}, {3, 4},
        {4, 4}, {4, 5},
        {5, 5}, {5, 6},
        {6, 6}, {6, 7},
        {7, 7}, {7, 8},
        {8, 8}, {8, 9},
        {9, 7}, {9, 9},
        {10, 6}, {10, 8},
        {11, 9}
    }};

    auto solution = dlx::DLX<10, 12, 23>::run(positions);
    REQUIRE(solution.has_value());
}

TEST_CASE("Small exact cover 4") {
    // Over 10 columns
    //   0 1 2 3 4 5 6 7 8 9
    // 0 1 1 0 0 0 0 0 0 0 0
    // 1 0 1 1 0 0 0 0 0 0 0
    // 2 0 0 1 1 0 0 0 0 0 0
    // 3 0 0 0 1 1 0 0 0 0 0
    // 4 0 0 0 0 1 1 0 0 0 0
    // 5 0 0 0 0 0 1 1 0 0 0
    // 6 0 0 0 0 0 0 1 1 0 0
    // 7 0 0 0 0 0 0 0 1 1 0
    // 8 0 0 0 0 0 0 0 0 1 1
    // 9 0 0 0 0 0 0 0 1 0 1
    // a 0 0 0 0 0 0 1 0 1 0
    // b 0 0 0 0 0 1 0 1 0 0
    // c 0 0 0 0 1 0 1 0 0 0
    // d 0 0 0 1 0 1 0 0 0 0
    // e 0 0 1 0 1 0 0 0 0 0
    // f 0 1 0 1 0 0 0 0 0 0
    // g 1 0 1 0 0 0 0 0 0 0
    // h 0 0 0 0 0 0 0 0 0 1
    constexpr dlx::position_array<35> positions {{
        {0, 0}, {0, 1},
        {1, 1}, {1, 2},
        {2, 2}, {2, 3},
        {3, 3}, {3, 4},
        {4, 4}, {4, 5},
        {5, 5}, {5, 6},
        {6, 6}, {6, 7},
        {7, 7}, {7, 8},
        {8, 8}, {8, 9},
        {9, 7}, {9, 9},
        {10, 6}, {10, 8},
        {11, 5}, {11, 7},
        {12, 4}, {12, 6},
        {13, 3}, {13, 5},
        {14, 2}, {14, 4},
        {15, 1}, {15, 3},
        {16, 0}, {16, 2},
        {17, 9}
    }};

    auto solution = dlx::DLX<10, 18, 35>::run(positions);
    REQUIRE(solution.has_value());
}
