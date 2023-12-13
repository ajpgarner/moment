/**
 * polynomial_localizing_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "polynomial_matrix.h"
#include "composite_matrix.h"

#include "matrix_system/indices/polynomial_localizing_matrix_index.h"
#include "matrix_system/standard_matrix_indices.h"

namespace Moment {

    /**
     * Polynomial localizing matrix: composed from monomial localizing matrices.
     */
    using PolynomialLocalizingMatrix = CompositeMatrixImpl<MatrixSystem,
                                                          ::Moment::PolynomialLocalizingMatrixIndex,
                                                          ::Moment::LocalizingMatrixIndices>;

}