/**
 * polynomial_indices.h
 *
 * Specializations of composite polynomial matrices to provide polynomial variants of localizing, commutator and
 * anti-commutator matrices in the Pauli scenario.
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix/polynomial_localizing_matrix.h"

#include "scenarios/pauli/pauli_index_collections.h"

#include "scenarios/pauli/matrices/monomial_commutator_matrix.h"
#include "scenarios/pauli/matrices/monomial_localizing_matrix.h"

namespace Moment::Pauli {
    class PauliMatrixSystem;

    /** Polynomial localizing matrix for Pauli scenario, composed of Pauli-scenario monomial localizing matrices. */
    using PolynomialLocalizingMatrix = CompositeMatrixImpl<PauliMatrixSystem,
                                                           Pauli::PolynomialLocalizingMatrixIndex,
                                                           Pauli::PauliLocalizingMatrixIndices>;

    /** Polynomial commutator matrix, composed of monomial commutator matrices. */
    using PolynomialCommutatorMatrix = CompositeMatrixImpl<PauliMatrixSystem,
                                                           Pauli::PolynomialCommutatorMatrixIndex,
                                                           Pauli::CommutatorMatrixIndices>;

    /** Polynomial anti-commutator matrix, composed of monomial anti-commutator matrices. */
    using PolynomialAnticommutatorMatrix = CompositeMatrixImpl<PauliMatrixSystem,
                                                               Pauli::PolynomialAnticommutatorMatrixIndex,
                                                               Pauli::AnticommutatorMatrixIndices>;
}