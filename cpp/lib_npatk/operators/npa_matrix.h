/**
 * npa_matrix.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operator_sequence.h"

#include <cassert>
#include <memory>
#include <span>

namespace NPATK {
    class Context;

    class NPAMatrix {
    private:
        const Context& context;

        const size_t hierarchy_level;

        size_t matrix_dimension;

        std::vector<OperatorSequence> matrix_data;

    public:
        NPAMatrix(const Context& the_context, size_t level);

        /**
         * @return The number of rows in the matrix. Matrix is square, so also the number of columns.
         */
        [[nodiscard]] size_t dimension() const noexcept {
            return matrix_dimension;
        }

        /**
         * @return The number of rows and columns in hte matrix. Matrix is square, so first and second are identical.
         */
        [[nodiscard]] auto dimensions() const noexcept {
            return std::make_pair(matrix_dimension, matrix_dimension);
        }

        /**
         * Return a view (std::span<OperatorSequence>) to the supplied row of the NPA matrix. Since std::span also
         * provides an operator[], it is possible to index using "myNPAMatrix[row][col]" notation.
         * @param row The index of the row to return.
         * @return A std::span<OperatorSequence> of the requested row.
         */
        auto operator[](size_t row) const noexcept {
            assert(row < this->matrix_dimension);
            auto iter_row_start = this->matrix_data.begin() + static_cast<ptrdiff_t>(row * this->matrix_dimension);
            return std::span<const OperatorSequence>(iter_row_start.operator->(), this->matrix_dimension);
        }

    };

}