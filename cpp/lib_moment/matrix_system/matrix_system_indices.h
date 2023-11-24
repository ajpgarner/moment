/**
 * matrix_system_indices.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_indices.h"
#include "localizing_matrix_index.h"
#include "polynomial_localizing_matrix_index.h"

#include "vector_index_storage.h"
#include "polynomial_index_storage.h"

#include "matrix_factories.h"

namespace Moment {

    class SymbolicMatrix;
    class MatrixSystem;
    class PolynomialMatrix;

    /**
     * Stores moment matrices by integer hierarchy depth.
     */
    using MomentMatrixIndices = MatrixIndices<SymbolicMatrix, size_t, VectorIndexStorage,
                                              MomentMatrixFactory, MatrixSystem>;

    /**
     * Stores monomial localizing matrices by localizing words and integer hierarchy depth.
     */
    using LocalizingMatrixIndices = MappedMatrixIndices<SymbolicMatrix, LocalizingMatrixIndex,
                                                        LocalizingMatrixFactory, MatrixSystem>;

    /**
     * Stores polynomial localizing matrices by polynomial and integer hierarchy depth.
     */
    using PolynomialLMIndices = MatrixIndices<PolynomialMatrix, PolynomialLMIndex, PolynomialIndexStorage,
                                              PolynomialLocalizingMatrixFactory, MatrixSystem>;

    /**
     * Stores substituted matrices by source index and rulebook index.
     */
    using SubstitutedMatrixIndices = MappedMatrixIndices<SymbolicMatrix, std::pair<ptrdiff_t, ptrdiff_t>,
                                                         SubstitutedMatrixFactory, MatrixSystem>;
}