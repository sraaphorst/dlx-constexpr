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
     * Note that the algorithm can be run entirely at compile time (but need not be),and only the first solution
     * found is returned. It would be ideal if we could return all solutions found, but I cannot ascertain a way to
     * do so in a constexpr setting. (We could, for example, allocate enough room for, say, 100 solutions and see
     * how many the algorithm finds.)
     *
     * Note that DLX is uninstantiable and thus just a class of static methods. They could be separated, but we keep
     * them so categorized so that the template sizes carry around, and also to enforce encapsulation, as there
     * are certain methods the user should not call.
     *
     * There are a number of improvements that could be made: these will be documented as issues in github.
     * 
     * @tparam NumCols the number of elements in the set to cover
     * @tparam NumRows the number of rows n the array of nodes
     * @tparam NumNodes the size of the array of nodes
     */
    template<size_t NumCols, size_t NumRows, size_t NumNodes>
    class DLX final {
    public:
        /** OUTPUT **/
        using solution = std::array<bool, NumRows>;

    private:
        /** INTERNAL DATA TYPES **/
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
        template <typename Data>
        static constexpr void coverColumn(Data &&state, index columnIdx) {
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
        template <typename Data>
        static constexpr void uncoverColumn(Data &&state, index columnIdx) {
            assert(0 <= columnIdx && columnIdx < header);

            // Reverse the removal of the rows from coverColumn.
            for (index i = state.U[columnIdx]; i != columnIdx; i = state.U[i]) {
                // Restore row i to the problem.
                for (index j = state.L[i]; j != i; j = state.L[j]) {
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
        template <typename Data, typename Solution>
        static constexpr void useRow(Data &&state, index rowIdx, Solution &&sol) {
            assert(rowIdx > header && rowIdx < HeaderSize + NumNodes);
            std::cerr << "Setting " << state.RM[rowIdx] << " to TRUE\n";
            sol[state.RM[rowIdx]] = true;

            // Cover all the columns in the row.
            index i  = rowIdx;
            do {
                coverColumn(std::forward<Data>(state), state.C[i]);
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
        template <typename Data, typename Solution>
        static constexpr void unuseRow(Data &&state, index rowIdx, solution &&sol) {
            assert(rowIdx > header && rowIdx < HeaderSize + NumNodes);
            sol[state.RM[rowIdx]] = false;
            std::cerr << "Unsetting " << state.RM[rowIdx] << " to FALSE\n";

            // Uncover all the columns in the row.
            index i = rowIdx;
            do {
                uncoverColumn(std::forward<Data>(state), state.C[i]);
                i = state.L[i];
            } while (i != rowIdx);
        }

        /**
         * Given a formulation produced by the run method, attempt to locate a solution to the exact cover
         * problem using backtracking on the rows. The general strategy is:
         * 1. If all columns are covered, solution found.
         * 2. Otherwise pick the column with the least rows (to minimize branching factor).
         * 3. Recurse over all possibilities of adding the rows covering thhe column to the answer.
         *
         * The algorithm manipulates the links between rows and columns as per Knuth's DLX paper to efficiently
         * expand / backtrack.
         *
         * @param state the DLX state
         * @param sol an array representing the solution
         * @return a solution if one exists, and nullopt if not
         */
        template <typename Data, typename Solution>
        static constexpr std::optional<solution> find_solution(Data &&state, Solution &&sol) {
            // Check to see if we have a complete solution, i.e if the header only loops to itself.
            // We could modify this to make a callback for solutions and then iterate over them, but
            // I'm not sure how we would do this with constexpr.
            if (state.R[header] == header)
                return sol;

            // Choose the column with the smallest number of rows to minimize the branching factor.
            index minColumnIndex = state.R[header];
            for (index i = state.R[minColumnIndex]; i != header; i = state.R[i])
                if (state.S[i] < state.S[minColumnIndex])
                    minColumnIndex = i;

            // If there ae no available rows to cover this column, we cannot extend.
            if (minColumnIndex == header || state.S[minColumnIndex] == 0)
                return std::nullopt;

            // Cover the column.
            coverColumn(std::forward<Data>(state), minColumnIndex);

            // Now extend the solution by trying each possible row in the column.
            for (index i = state.D[minColumnIndex]; i != minColumnIndex; i = state.D[i]) {
//                useRow(std::forward<Data>(state), i, std::forward<Solution>(sol));
                sol[state.RM[i]] = true;
                std::cerr << "- Setting " << state.RM[i] << " to TRUE\n";
                for (index j = state.R[i]; j != i; j = state.R[j])
                    coverColumn(std::forward<Data>(state), state.C[j]);

                // Recurse and see if we can find a solution.
                const auto soln = find_solution(std::forward<Data>(state), std::forward<Solution>(sol));
                if (soln.has_value())
                    return soln;

                // Reverse the operation.
//                unuseRow(std::forward<Data>(state), i, std::forward<Solution>(sol));
                sol[state.RM[i]] = false;
                std::cerr << "- Setting " << state.RM[i] << " to FALSE\n";
                for (index j = state.L[i]; j != i; j = state.L[j])
                    uncoverColumn(std::forward<Data>(state), state.C[j]);
            }

            // Uncover the column.
            uncoverColumn(std::forward<Data>(state), minColumnIndex);

            // If we reach this point, we could not find a row that leads to completion.
            return std::nullopt;
        }

        template <typename Data>
        static constexpr std::optional<solution> solve(Data &&state) {
            // Initialize the solution.
            solution sol{};
            for (auto &s: sol)
                s = false;
            return find_solution(std::forward<data>(state), sol);
        }

        static constexpr data init(const position_array<NumNodes> &positions) {
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
            return std::move(d);
        }

        static constexpr solution init_solution() {
            solution sol{};
            for (auto &s: sol)
                s = false;
            return std::move(sol);
        }

    public:
        DLX() = delete;

        /**
         * Given a set of "positions" of the form (r,c) indicating that element c is in subset r, run the
         * exact cover problem using Knuth's dancing links algorithm.
         *
         * @param positions list of the positions describing the subsets
         * @return the first solution found if one exists, or nullopt otherwise
         */
        static constexpr std::optional<solution> run(const position_array<NumNodes> &positions) {
            return find_solution(init(positions), init_solution());
        }

        /**
         * Given a set of "positions" of the form (r,c) indicating that element c is in subset r,
         * and a set of rows to force into the final solution, run the  exact cover problem using Knuth's
         * dancing links algorithm.
         *
         * @param positions list of the positions describing the subsets
         * @param fixed_rows a list of fixed rows (not offset by the header)
         * @return the first solution found if one exists, or nullopt otherwise
         */
        template<size_t NumFixedRows>
        static constexpr std::optional<solution> run(const position_array<NumNodes> &positions,
                const std::array<size_t, NumFixedRows> &fixed_rows) {
            auto state = init(positions);

            // Initialize the solution.
            auto sol = init_solution();

            for (const auto row: fixed_rows)
                useRow(state, row + HeaderSize, sol);
            return find_solution(state, sol);
        }
    };
}
