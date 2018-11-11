/**
 * TestSudoku.cpp
 *
 * By Sebastian Raaphorst, 2018.
 */

#include <catch.hpp>
#include <string_view>

#include <dlx_contexpr.h>

#include "TestSudoku.h"

using namespace sudoku;

TEST_CASE("Solve simple Sudoku") {
    constexpr fixing_array<27> fixings{{
        {0, 0, 1}, {0, 4, 8}, {0, 5, 9}, {0, 6, 4}, {0, 7, 5}, {0, 8, 7},
        {1, 0, 7}, {1, 1, 3}, {1, 2, 8},
        {2, 1, 4}, {2, 4, 1},
        {3, 2, 4}, {3, 4, 5}, {3, 6, 9}, {3, 8, 6},
        {5, 6, 7}, {5, 7, 2}, {5, 8, 8},
        {6, 1, 8}, {6, 5, 1},
        {7, 2, 7}, {7, 5, 8}, {7, 7, 9}, {7, 8, 5},
        {8, 1, 6}, {8, 4, 9}, {8, 6, 3}
    }};

    constexpr auto p = makeSudokuPositions<3>();
//    for (int i = 0; i < p.size(); ++i) {
//        if (i % 4 == 0)
//            std::cerr << '\n';
//        std::cerr << p[i].first << ":" << p[i].second << ' ';
//    }
    constexpr auto f = makeFixedRows(fixings);

    constexpr std::string_view board = "100089457738000000040010000004050906000000000000000728080001000007008095060090300";
    constexpr auto f2 = makeFixedCells<27>(board);
    REQUIRE(f == f2);

    for (const auto &e: f)
        std::cerr << e << std::endl;
    auto sol = dlx::DLX<324, 729, 2916>::run<27>(p, f);
    print_solution<3>(*sol);
    //constexpr auto sol = runSudoku<27>(fixings);
    REQUIRE(sol);
}