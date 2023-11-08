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
         * Construct a system of matrices with shared operators representing Pauli matrices.
         * @param context The Pauli  scenario.
         * * @param tolerance Floating point equivalence factor.
         */
        explicit PauliMatrixSystem(std::unique_ptr<class PauliContext> context, double tolerance = 1.0);

        /**
         * Construct a system of matrices with shared operators representing Pauli matrices.
         * @param qubit_count The number of qubits.
         * @param tolerance Floating point equivalence factor.
         */
        explicit PauliMatrixSystem(oper_name_t qubit_count, double tolerance = 1.0);

    public:
        ~PauliMatrixSystem();

        [[nodiscard]] std::string system_type_name() const final {
            return "Pauli Matrix System";
        }
    };
}
