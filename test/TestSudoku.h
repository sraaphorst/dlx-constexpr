/**
 * TestSudoku.h
 *
 * By Sebastian Raaphorst, 2018.
 */

#pragma once

#include <array>
#include <cstddef>
#include <tuple>

#include <dlx_contexpr.h>

namespace sudoku {
    /// A fixing on a Sudoku board, for cell (i, j) of digit k.
    using fixing = std::tuple<size_t, size_t, size_t>;

    template<size_t FixedRows>
    using fixing_array = std::array<fixing, FixedRows>;

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
    constexpr dlx::position_array<nodes> makeSudokuPositions() {
        using dlx_col_idx = int;

        // Determine the starting position for each of the four category classes above.
        dlx_col_idx offset = 0;

        // 1. Rows
        std::array<std::array<dlx_col_idx, digits>, rows> rowIndices{};
        for (dlx_col_idx i = 0; i < rows; ++i)
            for (dlx_col_idx n = 0; n < digits; ++n)
                rowIndices[i][n] = i * rows + digits + offset;
        offset += rows * digits;
        std::cerr << "offset1=" << offset << '\n';

        // 2. Columns
        std::array<std::array<dlx_col_idx, digits>, cols> colIndices{};
        for (dlx_col_idx j = 0; j < cols; ++j)
            for (dlx_col_idx n = 0; n < digits; ++n)
                rowIndices[j][n] = j * cols + digits + offset;
        offset += cols * digits;
        std::cerr << "offset2=" << offset << '\n';

        // 3. Grids. We could work out a formula here but it's just easier to increment since
        // the array has five dimensions.
        int position = 0;
        std::array<std::array<std::array<dlx_col_idx, digits>, N>, N> gridIndices{};
        for (dlx_col_idx x = 0; x < N; ++x)
            for (dlx_col_idx y = 0; y < N; ++y)
                for (dlx_col_idx n = 0; n < digits; ++n)
                    gridIndices[x][y][n] = position++ + offset;
        offset += position;
        std::cerr << "offset4=" << offset << '\n';

        // 4. Cell occupancy.
        std::array<std::array<dlx_col_idx, cols>, rows> occupancy{};
        for (dlx_col_idx i = 0; i < rows; ++i)
            for (dlx_col_idx j = 0; j < cols; ++j)
                occupancy[i][j] = i * rows + j + offset;
        offset += rows * cols;
        std::cerr << "offset5=" << offset << '\n';

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

    template<size_t N = 3,
            auto cols = N * N,
            auto digits = N * N,
            auto headerSize = 4 * (N * N * N * N) + 1>
    constexpr size_t toRow(const fixing &assignment) {
        const auto [row, col, digit] = assignment;

        // For each row, there are col * digit entries.
        // For each col, there are digit entries.
        // Thus, assignment is essentially a number written base N^2 that we convert to decimal.
        // We want a node index in the row, however, so multiply by 4.
        return headerSize + 4 * (row * cols * digits + col * digits + digit - 1);
    }

    template<size_t NumFixedRows, size_t N = 3>
    constexpr std::array<size_t, NumFixedRows> makeFixedRows(const fixing_array <NumFixedRows> &fixed) {
        std::array<size_t, NumFixedRows> rows{};
        for (size_t i = 0; i < NumFixedRows; ++i)
            rows[i] = toRow<N>(fixed[i]);
        return std::move(rows);
    }

    template<size_t NumFixedRows, size_t N = 3,
            auto rows = N * N,
            auto cols = N * N,
            auto digits = N * N,
            auto totalCols = 4 * (N * N * N * N),
            auto arrayRows = rows * cols * digits,
            auto arrayNodes = 4 * arrayRows>
    constexpr std::optional<std::array<bool, arrayRows>> runSudoku(const fixing_array<NumFixedRows> &fixed) {
        return dlx::DLX<totalCols, arrayRows, arrayNodes>::run(makeSudokuPositions<N>(),
                makeFixedRows<NumFixedRows, N>(fixed));
    }
}
