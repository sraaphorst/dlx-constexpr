/**
 * dlx.cpp
 *
 * By Sebastian Raaphorst, 2018.
 */

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <iostream>
#include <optional>
#include <tuple>

namespace dlx {
    /** INPUT PARAMETERS **/
    using index = std::size_t;

    using position = std::pair<int, int>;

    template<size_t NumNodes>
    using position_array = std::array<position, NumNodes>;

    /**
     * We represent an instance of the dancing links algorithm (an exact cover problem) which comprises:
     * 1. A header, representing the elements of the set to cover.
     * 2. A collection of rows, representing the subsets, with entries in the columns for the elements they cover.
     * We represent the set of things to cover as the set {0, 1, ... N-1} for a given N.
     * It is the responsibility of the user to map these values to meaningful data with respect to their problem.
     *
     * @tparam NumCols the number of elements in the set to cover
     * @tparam NumRows the number of rows n the array of nodes
     * @tparam NumNodes the size of the array of nodes
     */
    template<size_t NumCols, size_t NumRows, size_t NumNodes>
    struct DLX {
    private:
        /** DATA TYPES **/
        static constexpr size_t dim = NumCols + 1 + NumNodes;
        using direction = std::array<index, dim>;

        // The index of the header, the size of the header, the length of a column, and the type of column, which
        // determines if columns are primary (must be covered once) or secondary (covered at most once).
        static constexpr index header = NumCols;
        static constexpr size_t HeaderSize = NumCols + 1;
        using column_length = std::array<size_t, HeaderSize>;

        // A mapping from nodes to rows.
        // The header counts as the NumRows-th row.
        using row_mapping = std::array<index, dim>;

        /** OUTPUT **/
        using solution = std::array<bool, NumRows>;

        /**
         * The internal state used to model the problem.
         */
        struct data {
            // Header-specific: column type and length.
            column_length S{};

            // For all nodes: directional indices, including the column header.
            direction R{}; // right
            direction L{}; // left
            direction U{}; // up
            direction D{}; // down
            direction C{}; // column header

            // The coordinates of rows.
            row_mapping RM{};
        };

        /**
         * Covers a column, i.e. removes the column from the header, and then removes all rows that have entries
         * in the column from the problem.
         *
         * Note that executing:
         * 1. coverColumn(state, idx)
         * 2. uncoverColumn(state, idx)
         * should leave the problem unchanged.
         *
         * @param state the DLX state
         * @param columnIdx the index of the column
         */
        static constexpr void coverColumn(data &state, index columnIdx) {
            assert(0 <= columnIdx && columnIdx < header);

            // Remove the column from the header.
            state.L[state.R[columnIdx]] = state.L[columnIdx];
            state.R[state.L[columnIdx]] = state.R[columnIdx];

            // Iterate over the rows covered by this column and remove them.
            for (index i = state.D[columnIdx]; i != columnIdx; i = state.D[i]) {
                // Remove row i from the problem.
                for (index j = state.R[i]; j != i; j = state.R[j]) {
                    state.U[state.D[j]] = state.U[j];
                    state.D[state.U[j]] = state.D[j];
                    --state.S[state.C[j]];
                }
            }
        }

        /**
         * Uncovers a column, i.e. restores all rows that have entries in the column, and then restores the column
         * to the header.
         *
         * Note that executing:
         * 1. coverColumn(state, idx)
         * 2. uncoverColumn(state, idx)
         * should leave the problem unchanged.
         *
         * @param state the DLX state
         * @param columnIdx the index of the column
         */
        static constexpr void uncoverColumn(data &state, int columnIdx) {
            assert(0 <= columnIdx && columnIdx < header);

            // Reverse the removal of the rows from coverColumn.
            for (index i = state.U[columnIdx]; i != columnIdx; i = state.U[i]) {
                // Restore row i to the problem.
                for (index j = state.L[i]; j != i; j = state.L[i]) {
                    ++state.S[state.C[j]];
                    state.D[state.U[j]] = j;
                    state.U[state.D[j]] = j;
                }
            }

            // Restore the column to the header.
            state.R[state.L[columnIdx]] = columnIdx;
            state.L[state.R[columnIdx]] = columnIdx;
        }

        /**
         * Given a node index for a node in a row, include the row in the partial solution.
         *
         * Note that executing:
         * 1. useRow(state, idx, sol)
         * 2. unuseRow(state, idx, sol)
         * should leave the problem unchhanged.
         *
         * This method could be used to force certain rows into the solution.
         * This would be a simple modification to make.
         *
         * @param d the DLX state
         * @param rowIdx the index of a node in the row
         * @param sol the solution
         */
        static constexpr void useRow(data &state, index rowIdx, solution &sol) {
            assert(rowIdx > header && rowIdx < HeaderSize + NumNodes);
            sol[state.RM[rowIdx]] = true;

            // Cover all the columns in the row.
            index i  = rowIdx;
            do {
                coverColumn(state, state.C[i]);
                i = state.R[i];
                //} while (i != initialIdx);
            } while (i != rowIdx);
        }

        /**
         * Given a node index for a node in a row, remove the row from the partial solution.
         *
         * Note that executing:
         * 1. useRow(state, idx, sol)
         * 2. unuseRow(state, idx, sol)
         * should leave the problem unchhanged.
         *
         * @param d the DLX state
         * @param rowIdx the index of a node in the row
         * @param sol the solution
         */
        static constexpr void unuseRow(data &state, index rowIdx, solution &sol) {
            assert(rowIdx > header && rowIdx < HeaderSize + NumNodes);
            sol[state.RM[rowIdx]] = false;

            // Uncover all the columns in the row.
            index i = rowIdx;
            do {
                uncoverColumn(state, state.C[i]);
                i = state.L[i];
            } while (i != rowIdx);
        }

    public:

        /**
         * Given a set of "positions" of the form (r,c) indicating that element c is in subset r, run the
         * exact cover problem using Knuth's dancing links algorithm.
         *
         * @param positions list of the positions describing the subsets
         * @return the first solution found if one exists, or nullopt otherwise
         */
        static constexpr std::optional<solution> run(const position_array<NumNodes> &positions) {
            data d{};

            // Create the header data.
            for (index i = 0; i < HeaderSize; ++i) {
                //d.T[i] = columnTypes[i];
                d.U[i] = i;
                d.D[i] = i;
                d.C[i] = i;
                d.S[i] = 0;
                d.RM[i] = NumRows;
            }

            // Do the L-R linking of the columns.
            for (index i = 0; i <= header; ++i) {
                d.R[i] = (i + 1) % HeaderSize;
                d.L[i] = (i - 1 + HeaderSize) % HeaderSize;
            }

            // Now handle the rows.
            index rowIdx = 0;
            while (rowIdx < NumNodes) {
                // Find the start and end positions of this row.
                const index rowNumber = positions[rowIdx].first;

                const index rowStartIdx = rowIdx;
                index rowEndIdx = rowIdx;
                while (rowEndIdx < NumNodes && positions[rowEndIdx].first == rowNumber)
                    ++rowEndIdx;

                // The row goes from [rowStartIdx, rowEndIdx), so link the nodes.
                for (index idx = rowStartIdx; idx < rowEndIdx; ++idx) {
                    const auto[row, column] = positions[idx];
                    const index posIdx = HeaderSize + idx;

                    // Set the row and column for the new node.
                    d.C[posIdx] = column;
                    d.RM[posIdx] = rowNumber;

                    // Link to bottom of column, with up pointing to col's previous up, and down to header.
                    d.U[posIdx] = d.U[column];
                    d.D[posIdx] = column;

                    // Adjust the header to include the new entry.
                    d.D[d.U[column]] = posIdx;
                    d.U[column] = posIdx;

                    // Add 1 to the size of the column.
                    ++d.S[column];

                    // Link left to previous, if there is one, and right to first in row.
                    d.L[posIdx] = idx > rowStartIdx ? posIdx - 1 : posIdx;
                    d.R[posIdx] = rowStartIdx + HeaderSize;

                    // Adjust right and left links of this node to point to this node.
                    d.L[d.R[posIdx]] = posIdx;
                    d.R[d.L[posIdx]] = posIdx;
                }

                rowIdx = rowEndIdx;
            }

            // Initialize the solution.
            solution sol{};
            for (auto &s: sol)
                s = false;
            return find_solution(d, sol);
        }

        static constexpr std::optional<solution> find_solution(data &state, solution &sol) {
            // Check to see if we have a complete solution, i.e if the header only loops to itself.
            // We could modify this to make a callback for solutions and then iterate over them, but
            // I'm not sure how we would do this with constexpr.
            if (state.R[header] == header)
                return sol;

            // Choose the column with the smallest number of rows to minimize the branching factor.
            index minColumnIndex = state.R[header];
            for (index i = state.R[minColumnIndex]; i != header; ++i)
                if (state.S[i] < state.S[minColumnIndex])
                    minColumnIndex = i;

            // If there ae no available rows to cover this column, we cannot extend.
            if (state.S[minColumnIndex] == 0)
                return std::nullopt;

            // Cover the column.
            coverColumn(state, minColumnIndex);

            // Now extend the solution by trying each possible row in the column.
            for (index i = state.D[minColumnIndex]; i != minColumnIndex; i = state.D[i]) {
                useRow(state, i, sol);
//                sol[state.RM[i]] = true;
//                for (index j = state.R[i]; j != i; j = state.R[j])
//                    coverColumn(state, state.C[j]);

                // Recurse and see if we can find a solution.
                const auto soln = find_solution(state, sol);
                if (soln.has_value())
                    return soln;

                // Reverse the operation.
                unuseRow(state, i, sol);
//                sol[state.RM[i]] = false;
//                for (index j = state.L[i]; j != i; j = state.L[j])
//                    uncoverColumn(state, state.C[j]);
            }

            // If we reach this point, we could not find a row that leads to completion.
            return std::nullopt;
        }
    };
}
