/**
 * pauli_polynomial_localizing_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix/polynomial_localizing_matrix.h"
#include "scenarios/pauli/indices/polynomial_index.h"

#include "multithreading/multithreading.h"

namespace Moment {
    class RawPolynomial;
}

namespace Moment::Pauli {
    class PauliContext;

    class PolynomialLocalizingMatrix : public ::Moment::PolynomialLocalizingMatrix {
    public:
        using Index = Pauli::PolynomialLocalizingMatrixIndex;

        /** Pauli-scenario specific context */
        const PauliContext& pauli_context;

        /** Index with NN info */
        PolynomialLocalizingMatrixIndex nn_index;

    public:
        PolynomialLocalizingMatrix(const PauliContext& context, SymbolTable& symbols,
                                        const PolynomialFactory& factory,
                                        Index index,
                                        PolynomialLocalizingMatrix::ConstituentInfo&& constituents);

        PolynomialLocalizingMatrix(PauliMatrixSystem& system,
                                   NearestNeighbourIndex index,
                                   const std::string& raw_word_name,
                                   PolynomialLocalizingMatrix::ConstituentInfo&& constituents);


    public:
        /** Creates PolynomialLocalizingMatrix from raw polynomial */
        static std::unique_ptr<PolynomialLocalizingMatrix>
        create_from_raw(MaintainsMutex::WriteLock& write_lock, PauliMatrixSystem& system,
                        NearestNeighbourIndex index,  const RawPolynomial& raw_polynomials,
                        Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);
    };
}