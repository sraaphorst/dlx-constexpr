/**
 * comb_funcs.h
 *
 * By Sebastian Raaphorst, 2018.
 *
 * Simple combinatorial functions.
 */

#pragma once

#include <cstddef>

namespace cmath {
    using factype = size_t;

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
}

