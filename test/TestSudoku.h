/**
 * TestSudoku.h
 *
 * By Sebastian Raaphorst, 2018.
 */

#pragma once

#include <array>
#include <cstddef>

#include <dlx_contexpr.h>

namespace sudoku {
    /**
     * Create a formulation of a generic (N^2 by N^2) Sudoku board with entries from 1 to N^2.
     * In the case of the standard Sudoku board, N = 3.
     * The columns consist of:
     * 1. N^4 entries R_in, which represent that in row i, entry n appears.
     * 2. N^4 entries C_jn, which represent that in column j, entry n appears.
     * 3. N^4 enties G_xyijn, which represent that in subgrid (x,y)), at position (i,j), entry n appears.
     * 4. N^2 entries O_ij, which represent that in the board, cell (i,j) is occupied.
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

        // 2. Columns
        std::array<std::array<dlx_col_idx, digits>, cols> colIndices{};
        for (dlx_col_idx j = 0; j < cols; ++j)
            for (dlx_col_idx n = 0; n < digits; ++n)
                rowIndices[j][n] = j * cols + digits + offset;
        offset += cols * digits;

        // 3. Grids. We could work out a formula here but it's just easier to increment since
        // the array has five dimensions.
        int position = 0;
        const std::array<std::array<std::array<std::array<std::array<dlx_col_idx, digits>, N>, N>, N>, N> gridIndices{};
        for (dlx_col_idx x = 0; x < N; ++x)
            for (dlx_col_idx y = 0; y < N; ++y)
                for (dlx_col_idx i = 0; i < N; ++i)
                    for (dlx_col_idx j = 0; j < N; ++j)
                        for (dlx_col_idx n = 0; n < digits; ++n)
                            gridIndices[x][y][i][j][n] = position++ + offset;
        offset += position;

        // 4. Cell occupancy.
        const std::array<std::array<dlx_col_idx, cols>, rows> occupancy{};
        for (dlx_col_idx i = 0; i < rows; ++i)
            for (dlx_col_idx j = 0; j < cols; ++j)
                occupancy[i][j] = i * rows + j + offset;
        offset += rows * cols;

        //  Now we populate the position array, by creating N^6 rows trying every digit in
        // every row and every column.
        int row = 0;
        dlx::position_array<nodes> board{};
        for (dlx_col_idx i = 0; i < rows; ++i)
            for (dlx_col_idx j = 0; j < cols; ++j)
                for (dlx_col_idx n = 0; n < digits; ++n) {
                    // This represents cell (i,j) holding digit n, so create four entries.
                    board[4 * row + 0].first = row;
                    board[4 * row + 1].first = row;
                    board[4 * row + 2].first = row;
                    board[4 * row + 3].first = row;

                    // Now  find the columns for each, which correspond to the four cases above.
                    board[4 * row + 0].second = rowIndices[i][n];
                    board[4 * row + 1].second = colIndices[j][n];
                    board[4 * row + 2].second = gridIndices[i / N][j / N][i % N][j % N][n];
                    board[4 * row + 3].second = occupancy[i][j];

                    ++row;
                }

        return board;
    }
}
