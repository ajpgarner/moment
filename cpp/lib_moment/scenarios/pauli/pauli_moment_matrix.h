/**
 * pauli_moment_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "multithreading/multithreading.h"
#include "matrix/monomial_matrix.h"

#include "pauli_moment_matrix_indices.h"

namespace Moment {
    class SymbolTable;

    namespace Pauli {

        /**
         * Scalar extensions of monomial moment matrix
         */
        class PauliMomentMatrix : public MonomialMatrix {
        public:
            PauliMomentMatrix(SymbolTable& symbols, double zero_tolerance, const PauliMomentMatrixIndex& index,
                              Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);

        };

    }
}