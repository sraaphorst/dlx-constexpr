/**
 * TestSteinerTripleSystem.cpp
 *
 * By Sebastian Raaphorst, 2018.
 */

#include <iostream>
#include <optional>

#include <catch.hpp>
#include <dlx_contexpr.h>
#include "comb_funcs.h"

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

/**
 * Create a formulation of a t-(v, k, 1) design.
 * The columns are the t-sets, of which there are C(v, t).
 * The rows are the k-sets, with position <r,c> indicating that the rth k-set contains thhe cth t-set.
 *
 * We have:
 * 1. C(v, t) cols
 * 2. C(v, k) rows
 * 3. C(v, k) * C(k, t) entries.
 */
template<size_t v, size_t k, size_t t,
        size_t cols = nCr(v, t),
        size_t rows = nCr(v, k),
        size_t nodes_per_row = nCr(k, t),
        size_t nodes = rows * nodes_per_row>
constexpr dlx::position_array<nodes> makeDesignPositions() {
    dlx::position_array<nodes> array{};

    // Keep track of the current node.
    int node = 0;
    for (int row = 0; row < rows; ++row) {
        const auto kset = unrankKSubset<v, k>(row);
        for (int col = 0; col < nodes_per_row; ++col) {
            const auto tsetIdx = unrankKSubset<k, t>(col);

            // We need to translate tsetIdx into a subset of k-set.
            std::array<factype, t> tset{};
            for (int i = 0; i < t; ++i)
                tset[i] = kset[tsetIdx[i]];
            const auto tsetRk = rankKSubset<k, t>(tset);

            // Now we add a node, namely (row, tsetRk).
            // In order to mark this methhod constexpr, we need to assign to first and second individually.
            array[node].first = row;
            array[node].second = tsetRk;
            ++node;
        }
    }

    return std::move(array);
}

TEST_CASE("STS(7)") {
    constexpr auto v = 7;
    constexpr auto k = 3;
    constexpr auto t = 2;
    constexpr auto cols = nCr(v, t);
    constexpr auto rows = nCr(v, k);
    constexpr auto nodes_per_row = nCr(k, t);
    constexpr auto nodes = rows * nodes_per_row;
    constexpr auto positions = makeDesignPositions<v, k, t>();

    // And solve, all constexpr!
    constexpr auto solution = dlx::DLX<cols, rows, nodes>::run(positions);

    REQUIRE(solution.has_value());
}