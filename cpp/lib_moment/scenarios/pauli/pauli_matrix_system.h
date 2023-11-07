/**
 * pauli_matrix_system.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_system/matrix_system.h"

namespace Moment::Pauli {

    class PauliContext;
    class PauliMatrixSystem : public MatrixSystem {
    public:
        const class PauliContext& pauliContext;

    public:
        /**
         * Construct a system of matrices with shared operators.
         * @param context The operator scenario.
         */
        explicit PauliMatrixSystem(std::unique_ptr<class PauliContext> context, double tolerance = 1.0);

    public:
        ~PauliMatrixSystem();

    };
}
