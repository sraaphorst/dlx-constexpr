/**
 * comb_funcs.h
 *
 * By Sebastian Raaphorst, 2018.
 *
 * Simple combinatorial functions.
 */

#pragma once

#include <array>
#include <cstddef>
#include <iostream>
#include <optional>

#include <dlx_contexpr.h>

namespace cmath {
    using factype = unsigned long long;

    constexpr factype nCr(factype n, factype r) {
        if (n < r)
            return 0;

        factype bigger = n - r > r ? n - r : r;

        factype f = 1;
        for (factype i = n; i > bigger; --i)
            f *= i;
        for (factype i = (n - bigger); i >= 1; --i)
            f /= i;
        return f;
    }

    template<factype N, factype K>
    constexpr factype cpow() {
        if constexpr(K == 0)
            return 1;
        return N * cpow<N, K-1>();
    }

    template<factype v, factype k>
    constexpr factype rankKSubset(const std::array<factype, k> &kset) {
        factype r = nCr(v, k);
        for (int i = 0; i < k; ++i)
            r -= nCr(v - kset[i] - 1, k - i);
        return r - 1;
    }


    template<factype v, factype k>
    constexpr std::array<factype, k> unrankKSubset(factype rank) {
        std::array<factype, k> kset{};

        factype vi = nCr(v, k);
        factype j = v;
        factype ki = k;
        factype s = rank + 1;

        for (factype i = 0; i < k - 1; ++i) {
            while (s > vi - nCr(j, ki))
                --j;
            kset[i] = v - j - 1;

            s += nCr(j + 1, ki) - vi;
            --ki;
            vi = nCr(j, ki);
        }

        kset[k - 1] = v + s - vi - 1;
        return std::move(kset);
    }

    template<factype v, factype k>
    constexpr std::array<factype, k> succKSubset(std::array<factype, k> kset) {
        for (factype i = k-1; i >= 0; --i) {
            ++kset[i];
            if (kset[i] < v && kset[i] + (k - i) <= v) {
                for (factype j = i + 1; j < k; ++j)
                    kset[j] = kset[i] + j - i;
                break;
            }
        }

        return kset;
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
                const auto tsetRk = rankKSubset<v, t>(tset);

                // Now we add a node, namely (row, tsetRk).
                // In order to mark this methhod constexpr, we need to assign to first and second individually.
                array[node].first = row;
                array[node].second = tsetRk;
                ++node;
            }
        }

        return std::move(array);
    }

    template<size_t v, size_t k, size_t t,
            auto cols = nCr(v, t),
            auto rows = nCr(v, k),
            auto nodes_per_row = nCr(k, t),
            auto nodes = rows * nodes_per_row>
    constexpr std::optional<std::array<bool, rows>> run_t_design() {
        const auto positions = cmath::makeDesignPositions<v, k, t>();

        // And solve, all constexpr!
        return dlx::DLX<cols, rows, nodes>::run(positions);
    }

    template<size_t v, size_t k, size_t t,
            auto rows = nCr(v, k)>
    void print_solution(const std::optional<std::array<bool, rows>> &solution) {
        for (int i = 0; i < rows; ++i)
            if ((*solution)[i]) {
                auto kset = unrankKSubset<v, k>(i);
                for (const auto &e: kset)
                    std::clog << e << ' ';
                std::clog << '\n';
            }
        std::flush(std::clog);
    }
}

