/**
 * TestSudoku.h
 *
 * By Sebastian Raaphorst, 2018.
 */

#pragma once

#include <array>
#include <cstddef>
#include <iostream>
#include <string_view>
#include <tuple>

#include <dlx_contexpr.h>

namespace sudoku {
    /// A fixing on a Sudoku board, for cell (i, j) of digit k.
    using fixing = std::tuple<size_t, size_t, size_t>;

    /// An array of fixings.
    template<size_t FixedRows>
    using fixing_array = std::array<fixing, FixedRows>;

    /// The product of taking fixings / a stringview and running it through makeFixedCells.
    template<size_t NumFixedRows>
    using fixed_rows = std::array<size_t, NumFixedRows>;

    /// A solution to a Sudoku problem. Yes, we want N^6, as there are N^6 rows in the problem.
    template<size_t N=3>
    using solution = std::array<bool, N * N * N * N * N * N>;

    /// The solution extracted to a grid / board.
    template<size_t N=3>
    using board = std::array<std::array<size_t, N * N>, N * N>;

    /**
     * Create a formulation of a generic (N^2 by N^2) Sudoku board with entries from 1 to N^2.
     * In the case of the standard Sudoku board, N = 3.
     * The columns consist of:
     * 1. N^4 entries R_in, which represent that in row i, entry n appears.
     * 2. N^4 entries C_jn, which represent that in column j, entry n appears.
     * 3. N^4 enties G_xyn, which represent that in subgrid (x,y),  entry n appears.
     * 4. N^4 entries O_ij, which represent that in the board, cell (i,j) is occupied.
     *    (Without these constraints, you could have the algorithm allow multiple entries per cell.)
     *
     * We have N^6 rows in the DLX problem, say R_ijn which represent the digit n appearing in
     * cell (i,j) of the board. Each row has exactly four entries (one for each of the four
     * classes appearing above.
     */
    template<size_t N = 3,
            auto rows = N * N,
            auto cols = N * N,
            auto digits = N * N,
            auto nodes = 4 * rows * cols * digits>
    constexpr dlx::position_array<nodes> makeSudokuPositions() noexcept {
        using dlx_col_idx = int;

        // Determine the starting position for each of the four category classes above.
        dlx_col_idx offset = 0;

        // 1. Rows
        std::array<std::array<dlx_col_idx, digits>, rows> rowIndices{};
        for (dlx_col_idx i = 0; i < rows; ++i)
            for (dlx_col_idx n = 0; n < digits; ++n)
                rowIndices[i][n] = i * rows + n + offset;
        offset += rows * digits;

        // 2. Columns
        std::array<std::array<dlx_col_idx, digits>, cols> colIndices{};
        for (dlx_col_idx j = 0; j < cols; ++j)
            for (dlx_col_idx n = 0; n < digits; ++n)
                colIndices[j][n] = j * cols + n + offset;
        offset += cols * digits;

        // 3. Grids. We could work out a formula here but it's just easier to increment since
        // the array has five dimensions.
        int position = 0;
        std::array<std::array<std::array<dlx_col_idx, digits>, N>, N> gridIndices{};
        for (dlx_col_idx x = 0; x < N; ++x)
            for (dlx_col_idx y = 0; y < N; ++y)
                for (dlx_col_idx n = 0; n < digits; ++n)
                    gridIndices[x][y][n] = position++ + offset;
        offset += position;

        // 4. Cell occupancy.
        std::array<std::array<dlx_col_idx, cols>, rows> occupancy{};
        for (dlx_col_idx i = 0; i < rows; ++i)
            for (dlx_col_idx j = 0; j < cols; ++j)
                occupancy[i][j] = i * rows + j + offset;

        // Now we populate the position array, by creating N^6 rows trying every digit in
        // every row and every column.
        int row = 0;
        dlx::position_array<nodes> array{};
        for (dlx_col_idx i = 0; i < rows; ++i)
            for (dlx_col_idx j = 0; j < cols; ++j)
                for (dlx_col_idx n = 0; n < digits; ++n) {
                    // This represents cell (i,j) holding digit n, so create four entries.
                    array[4 * row + 0].first = row;
                    array[4 * row + 1].first = row;
                    array[4 * row + 2].first = row;
                    array[4 * row + 3].first = row;

                    // Now  find the columns for each, which correspond to the four cases above.
                    array[4 * row + 0].second = rowIndices[i][n];
                    array[4 * row + 1].second = colIndices[j][n];
                    array[4 * row + 2].second = gridIndices[i / N][j / N][n];
                    array[4 * row + 3].second = occupancy[i][j];

                    ++row;
                }

        return std::move(array);
    }

    /**
     * Giving a single fixing assignment of the form of a triple (i, j, d) where d is the digit fixed at
     * row i, column j, this transforms it into a form usable by DLX.
     *
     * @tparam N the size parameter of the Sudoku
     * @param assignment the fixing
     * @return an index usable by DLX
     */
    template<size_t N = 3,
            const auto cols = N * N,
            const auto digits = N * N>
    constexpr size_t toRow(const fixing &assignment) noexcept {
        const auto [row, col, digit] = assignment;

        // For each row, there are col * digit entries.
        // For each col, there are digit entries.
        // Thus, assignment is essentially a number written base N^2 that we convert to decimal.
        // We want a node index in the row, however, so multiply by 4.
        return 4 * (row * cols * digits + col * digits + digit - 1);
    }

    /**
     * Giving a fixing array, transform it using toRow to a format usable by DLX.
     *
     * @tparam NumFixedRows the number of fixings in fixed: these should all have unique row and column
     * @tparam N the size parameter of the Sudoku
     * @param fixed the fixing array
     * @return an array usable by DLX
     */
    template<size_t NumFixedRows, size_t N = 3>
    constexpr std::array<size_t, NumFixedRows> makeFixedCells(const fixing_array <NumFixedRows> &fixed) noexcept {
        std::array<size_t, NumFixedRows> rows{};
        for (size_t i = 0; i < NumFixedRows; ++i)
            rows[i] = toRow<N>(fixed[i]);
        return std::move(rows);
    }

    /**
     * A constexpr version of toupper for use with the string_view version of makeFixedCells.
     * @param c the char
     * @return uppercase version of the char
     */
    constexpr auto cToUpper(char c) noexcept {
        return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;
    }

    /**
     * Given an N^4 representation of the Sudoku problem as a string_view, where 0 indicates an unfixed cell,
     * this function parses and creates the fixed rows to be usd by DLX.
     *
     * @tparam NumFixedRows the number of non-zero entries in sv
     * @tparam N the size parameter of the Sudoku
     * @param sv a string_view representing the partial game, with 0s for unfixed cells
     * @return an array usable by DLX
     */
    template<size_t NumFixedRows, size_t N = 3>
    constexpr fixed_rows<NumFixedRows> makeFixedCells(const std::string_view &sv) noexcept {
        fixed_rows<NumFixedRows> rows{};
        int pos = 0;
        for (size_t i = 0; i < 81; ++i) {
            if (sv[i] == '0')
                continue;

            // We allow values > 9 using alphabetical representation.
            // For example A = 10, B = 11, C = 12.
            const auto c = static_cast<size_t>(cToUpper(sv[i]));
            const auto val = (c >= 'A' && c <= 'Z') ? c + 10 - 'A' : c - '0';
            rows[pos++] = toRow<N>({i / 9, i % 9, val});
        }
        return rows;
    }

    /**
     * A convenience method to invoke the DLX solver for Sudoku, since the parameters to pass are quite
     * involved. This takes a representation of a partial Suroku (in this case, a set of triples (i, j, d) which
     * state that digit d occurs in position row i, column j).
     *
     * It then returns a solution, if one exists.
     *
     * @tparam NumFixedRows the number of fixed entries in sv
     * @tparam N the size parameter of the Sudoku
     * @param sv a string_view representing the partial game, with 0s for unfixed cells
     * @return
     */
    template<size_t NumFixedRows, size_t N = 3,
            const auto rows = N * N,
            const auto cols = N * N,
            const auto digits = N * N,
            const auto totalCols = 4 * (N * N * N * N),
            const auto arrayRows = rows * cols * digits,
            const auto arrayNodes = 4 * arrayRows>
    constexpr std::optional<solution<N>> runSudoku(const fixing_array<NumFixedRows> &fixed) noexcept {
        return dlx::DLX<totalCols, arrayRows, arrayNodes>::run(makeSudokuPositions<N>(),
                makeFixedCells<NumFixedRows, N>(fixed));
    }

    /**
     * A convenience method to invoke the DLX solver for Sudoku, since the parameters to pass are quite
     * involved. This takes a representation of a partial Suroku (in this case, a string of N^4 valid characters
     * in length), parses it, and then formulates the problem and returns a solution, if one exists.
     *
     * @tparam NumFixedRows the number of fixed entries in sv
     * @tparam N the size parameter of the Sudoku
     * @param sv a string_view representing the partial game, with 0s for unfixed cells
     * @return fist solution found, if one exists
     */
    template<size_t NumFixedRows, size_t N = 3,
            const auto rows = N * N,
            const auto cols = N * N,
            const auto digits = N * N,
            const auto totalCols = 4 * (N * N * N * N),
            const auto arrayRows = rows * cols * digits,
            const auto arrayNodes = 4 * arrayRows>
    constexpr std::optional<solution<N>> runSudoku(const std::string_view &sv) noexcept {
        return dlx::DLX<totalCols, arrayRows, arrayNodes>::run(makeSudokuPositions<N>(),
                                                               makeFixedCells<NumFixedRows, N>(sv));
    }

    /**
     * Extract the solution as a grid / board from the solution provided by DLX.
     *
     * The template sizes represent side, num rows, and extractors.
     *
     * @tparam N the size parameter of the Sudoku
     * @param sol a solution returned by DLX
     * @return a board extracted from the Sudoku problem
     */
    template<size_t N = 3,
            const auto side = N * N,
            const auto N4 = side * side,
            const auto N6 = side * N4>
    constexpr board<N> extractBoard(const solution<N> &sol) noexcept {
        board<N> b{};

        // Yes, N^6.
        for (size_t i = 0; i < N6; ++i) {
            if (!sol[i]) continue;

            // We need to shift the digits from [0,8] to [1,9].
            size_t row = i / N4;
            size_t col = (i % N4) / side;
            size_t digit = (i % N4) % side + 1;
            b[row][col] = digit;
        }

        return b;
    }

    /**
     * Display the Sudoku grid to clog.
     * @tparam N the size parameter of the Sudoku
     * @param b the Sudoku board / grid
     */
    template<size_t N = 3,
            const auto side = N * N>
    void print_board(const board<N> &b) noexcept {
        for (size_t i = 0; i < side; ++i) {
            for (size_t j = 0; j < side; ++j)
                std::clog << b[i][j] << ' ';
            std::clog << '\n';
        }
        std::flush(std::clog);
    }

    /**
     * Extract the solution into a board and print it in one step.
     * @tparam N thhe size parameter of the Sudoku
     * @param sol a solution returned by DLX
     */
    template<size_t N = 3>
    void print_solution(const solution<N> &sol) noexcept {
        print_board<N>(extractBoard<N>(sol));
    }
}
