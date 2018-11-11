/**
 * TestSteinerTripleSystem.cpp
 *
 * By Sebastian Raaphorst, 2018.
 */

#include <iostream>
#include <optional>

#include <catch.hpp>
#include <dlx_contexpr.h>
#include "TestTDesign.h"

using namespace cmath;

//template<size_t T, size_t V, size_t K>

TEST_CASE("C(11, 4)") {
    constexpr auto f1 = nCr(11, 4);
    REQUIRE(f1 == 330);

    constexpr auto f2 = nCr(11, 7);
    REQUIRE(f2 == 330);
}

TEST_CASE("Ranking and unranking 3-sets of [8]") {
    constexpr auto v = 8;
    constexpr auto k = 3;
    constexpr auto num = nCr(v, k);
    for (factype rk = 0; rk < num; ++rk)
        REQUIRE(rankKSubset<v, k>(unrankKSubset<v,k>(rk)) == rk);
}

TEST_CASE("Successor of 4-sets of [8]") {
    constexpr auto v = 8;
    constexpr auto k = 4;
    constexpr auto num = nCr(v, k);
    for (factype rk = 1; rk < num; ++rk) {
        REQUIRE(succKSubset<v, k>(unrankKSubset<v, k>(rk - 1)) == unrankKSubset<v, k>(rk));
    }
}

TEST_CASE("Successor of 2-sets of [10]") {
    constexpr auto v = 10;
    constexpr auto k = 2;
    constexpr auto num = nCr(v, k);
    for (factype rk = 1; rk < num; ++rk) {
        REQUIRE(succKSubset<v, k>(unrankKSubset<v, k>(rk - 1)) == unrankKSubset<v, k>(rk));
    }
}

TEST_CASE("Successor of 1-sets of [10]") {
    constexpr auto v = 10;
    constexpr auto k = 1;
    constexpr auto num = nCr(v, k);
    for (factype rk = 1; rk < num; ++rk) {
        REQUIRE(succKSubset<v, k>(unrankKSubset<v, k>(rk - 1)) == unrankKSubset<v, k>(rk));
    }
}


TEST_CASE("STS(7)") {
    constexpr auto solution = cmath::run_t_design<7, 3, 2>();
    REQUIRE(solution);
    cmath::print_solution<7, 3, 2>(solution);
}

TEST_CASE("STS(15)") {
    constexpr auto solution = cmath::run_t_design<15, 3, 2>();
    REQUIRE(solution);
    cmath::print_solution<15, 3, 2>(solution);
}

TEST_CASE("No STS(8)") {
    constexpr auto solution = cmath::run_t_design<8, 3, 2>();
    REQUIRE(!solution);
}

TEST_CASE("SQS(8)") {
    constexpr auto solution = cmath::run_t_design<8, 4, 3>();
    REQUIRE(solution);
    cmath::print_solution<8, 4, 3>(solution);
}
TEST_CASE("SQS(10)") {
    constexpr auto solution = cmath::run_t_design<10, 4, 3>();
    REQUIRE(solution);
    cmath::print_solution<10, 4, 3>(solution);
}