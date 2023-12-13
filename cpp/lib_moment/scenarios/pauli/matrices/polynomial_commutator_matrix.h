/**
 * polynomial_commutator_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "monomial_commutator_matrix.h"
#include "scenarios/pauli/indices/polynomial_index.h"

#include "matrix/composite_matrix.h"
#include "matrix/polynomial_localizing_matrix.h"

#include "multithreading/maintains_mutex.h"
#include "multithreading/multithreading.h"

namespace Moment {
    class RawPolynomial;
}

namespace Moment::Pauli {

    class PauliContext;

    class PolynomialCommutatorMatrix : public CompositeMatrix {
    public:
        using Index = PolynomialCommutatorMatrixIndex;

        const PauliContext& pauli_context;
        const PolynomialCommutatorMatrixIndex index;

    public:
        PolynomialCommutatorMatrix(const PauliContext& context, SymbolTable& symbols,
                                   const PolynomialFactory& factory,
                                   Index index, CompositeMatrix::ConstituentInfo&& constituents);

        PolynomialCommutatorMatrix(PauliMatrixSystem& system, NearestNeighbourIndex index,
                                   const std::string& raw_word_name, CompositeMatrix::ConstituentInfo&& constituents);

        /** Creates PolynomialLocalizingMatrix from raw polynomial */
        static std::unique_ptr<PolynomialCommutatorMatrix>
        create_from_raw(MaintainsMutex::WriteLock& write_lock, PauliMatrixSystem& system,
                        NearestNeighbourIndex index,  const RawPolynomial& raw_polynomials,
                        Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);
    };

    class PolynomialAnticommutatorMatrix : public CompositeMatrix {
    public:
        using Index = PolynomialAnticommutatorMatrixIndex;

        const PauliContext& pauli_context;
        const PolynomialAnticommutatorMatrixIndex index;

    public:
        PolynomialAnticommutatorMatrix(const PauliContext& context, SymbolTable& symbols,
                                       const PolynomialFactory& factory,
                                       Index index, CompositeMatrix::ConstituentInfo&& constituents);


        PolynomialAnticommutatorMatrix(PauliMatrixSystem& system, NearestNeighbourIndex index,
                                       const std::string& raw_word_name,
                                       CompositeMatrix::ConstituentInfo&& constituents);

        /** Creates PolynomialLocalizingMatrix from raw polynomial */
        static std::unique_ptr<PolynomialAnticommutatorMatrix>
        create_from_raw(MaintainsMutex::WriteLock& write_lock, PauliMatrixSystem& system,
                        NearestNeighbourIndex index, const RawPolynomial& raw_polynomials,
                        Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);
    };
}