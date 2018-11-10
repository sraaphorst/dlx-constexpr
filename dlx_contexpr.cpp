/**
 * dlx.cpp
 *
 * By Sebastian Raaphorst, 2018.
 */

#include <algorithm>
#include <array>
#include <iostream>
#include <tuple>

namespace dlx {
    using position = std::pair<int, int>;

    template<size_t NumNodes>
    using position_array = std::array<position, NumNodes>;

    namespace details {
        // Since std::count_if is annoyingly not constexpr, we need a constexpr implementation of how to count the
        // number of pairs {r, c} in an array where c has a set value.
        constexpr size_t inColumn(const position &p, const int column) {
            return (p.second == column) ? 1 : 0;
        }

        // Count the number of column positions in this column.
        template<size_t M>
        constexpr size_t countColumnNodes(const position_array<M> &parr, int column) {
            size_t ctr = 0;
            for (const auto &p: parr)
                ctr += inColumn(p, column);
            return ctr;
        }
    }
    /**
     * We represent an instance of the dancing links algorithm (an exact cover problem) which comprises:
     * 1. A header, representing the elements of the set to cover.
     * 2. A collection of rows, representing the subsets, with entries in the columns for the elements they cover.
     * We represent the set of things to cover as the set {0, 1, ... N-1} for a given N.
     * It is the responsibility of the user to map these values to meaningful data with respect to their problem.
     *
     * @tparam N the number of elements in the set to cover
     * @tparam M the size of the array of nodes
     */
    template<int N, int M>
    struct DLX {
        // Comparator for columns.
        static constexpr auto columnComparator = [](const position &p1, const position &p2) {
            return p1.second < p2.second; };

        struct dlx_item {
            // The index of an item into an external array.
            int arrayidx;

            dlx_item() = delete;

            constexpr explicit dlx_item(int arrayidx) : arrayidx{arrayidx} {}
        };

        /**
         * A header, i.e. an element to cover.
         */
        struct dlx_header : dlx_item {
            // The ID of the header, which, in the initial problem, is its index in the array of headers.
            // As headers are removed, this will no longer be the case.
            int id = -1;

            // Links to left and right headers.
            int lidx = -1;
            int ridx = -1;

            // Links to the top and bottom dlx_nodes in the column, as indices into the array of nodes.
            int uidx = -1;
            int didx = -1;

            // The number of row candidates for this header.
            size_t rowcount = 0;

            constexpr dlx_header(): dlx_item{0} {}
            constexpr dlx_header(int arrayidx, int id, int lidx, int ridx, size_t rowcount, int uidx, int didx) :
                    dlx_item{arrayidx}, id{id}, lidx{lidx}, uidx{uidx}, rowcount{rowcount}, ridx{ridx}, didx{didx} {}
            constexpr dlx_header &operator=(const dlx_header&) noexcept = default;
        };

        /**
         * A single node in the collection of rows.
         */
        struct dlx_node : dlx_item {
            // The row and column position that this element represents.
            int row = -1;
            int column = -1;

            // Indices of the nodes to the left, right, up, and down nodes.
            int lidx = 0;
            int ridx = 0;
            int uidx = 0;
            int didx = 0;

            // Index of the header node.
            int hdridx = -1;

            constexpr dlx_node(): dlx_item{0} {};
            constexpr dlx_node(int arrayidx, int rowpos, int colpos,
                               int hdridx, int lidx, int ridx, int uidx, int didx) :
                    dlx_item{arrayidx}, row{rowpos}, column{colpos},
                    hdridx{hdridx}, lidx{lidx}, ridx{ridx}, uidx{uidx}, didx{didx} {}
            constexpr dlx_node &operator=(const dlx_node&) = default;
        };

        std::array<dlx_header, N> headers;
        std::array<dlx_node, M> nodes;

        /**
         * Representation of the problem: an array of pairs representing the row and column
         * positions of the nodes. Assumes that positions is sorted first by row, and then by column.
         * The behaviour is undefined if this is not the case.
         * @param positions array of positions, sorted by row and then column.
         */
        constexpr explicit DLX(const position_array<M> &positions) {
            static_assert(N > 0, "must have at least one column");
            static_assert(M > 0, "must have at least one position");

            // Create the basic headers and nodes.
            for (int i=0; i < N; ++i)
                headers[i] = dlx_header{i, i,
                                        (i - 1 + N) % N, (i + 1) % N,
                                        details::countColumnNodes(positions, i),
                                        0, 0};

            for (int i=0; i < M; ++i)
                nodes[i] = dlx_node{i, positions[i].first, positions[i].second,
                                    positions[i].second, 0, 0, 0, 0};

            // Find the minimum and maximum columns.
            const auto minCol = std::min_element(std::cbegin(positions), std::cend(positions), columnComparator)->second;
            const auto maxCol = std::max_element(std::cbegin(positions), std::cend(positions), columnComparator)->second;

            // Link the rows left-to-right. This is done in O(3n).
            int ridx = 0;
            while (ridx < M) {
                // Find the start and end positions of this row.
                const int row = nodes[ridx].row;
                int endRowIdx = ridx;
                while (endRowIdx < M && nodes[endRowIdx].row == row)
                    ++endRowIdx;

                // The row goes from [idx, endRowIdx), so link the nodes.
                for (int i = ridx; i < endRowIdx; ++i) {
                    nodes[i].lidx = (i - 1 < ridx) ? endRowIdx - 1 : i - 1;
                    nodes[i].ridx = (i + 1 == endRowIdx) ? ridx : i + 1;
                }

                // Begin the next row.
                ridx = endRowIdx;
            }

            // Link the rows top-to-bottom, which is more complicated.
            // Iterate over the headers and gather the nodes.
            // This is considerably more complicated, on the order of O(M^2).
            for (int column = 0; column < N; ++column) {
                // Note that if numNodes == 0, there is no solution, but we do not deliberately enforce this here.
                const int numNodes = headers[column].rowcount;
                if (numNodes == 0)
                    continue;

                int firstNode = -1;
                int lastNode = -1;
                int prevNode = -1;
                int foundNodes = 0;

                // Find all numNodes in the column.
                for (int idx = 0; idx < M && foundNodes < numNodes; ++idx) {
                    const auto &n = nodes[idx];
                    if (n.column != column)
                        continue;

                    ++foundNodes;
                    if (firstNode == -1)
                        firstNode = idx;
                    if (foundNodes == numNodes)
                        lastNode = idx;

                    // If there was a previous node, link it to this node.
                    if (prevNode != -1) {
                        nodes[prevNode].didx = idx;
                        nodes[idx].uidx = prevNode;
                    }

                    prevNode = idx;
                }

                // Now link the top and bottom node.
                nodes[firstNode].uidx = lastNode;
                nodes[lastNode].didx  = firstNode;

                // Link the column header.
                headers[column].didx  = firstNode;
                headers[column].uidx  = lastNode;
            };
        }
    };
}

int main() {
    // Here is the structure of the test:
    //    0 1 2 3 4 5
    // r0 1 0 1 0 1 0
    // r1 1 1 0 1 0 1
    // r2 0 1 0 1 0 0
    // r3 0 0 0 0 0 1
    constexpr dlx::position_array<10> positions {
        /** r0 **/ std::make_pair(0, 0), std::make_pair(0, 2), std::make_pair(0, 4),
        /** r1 **/ std::make_pair(1, 0), std::make_pair(1, 1), std::make_pair(1, 3), std::make_pair(1, 5),
        /** r2 **/ std::make_pair(2, 1), std::make_pair(2, 3),
        /** r3 **/ std::make_pair(2, 5)
    };

    constexpr dlx::DLX<6, 10> problem{positions};

//    dlx::position_array<10> positions {
//            /** r0 **/ std::make_pair(0, 0), std::make_pair(0, 2), std::make_pair(0, 4),
//            /** r1 **/ std::make_pair(1, 0), std::make_pair(1, 1), std::make_pair(1, 3), std::make_pair(1, 5),
//            /** r2 **/ std::make_pair(2, 1), std::make_pair(2, 3),
//            /** r3 **/ std::make_pair(2, 5)
//    };
//
//    dlx::DLX<6, 10> problem{positions};
}