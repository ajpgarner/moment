/**
 * pauli_polynomial_localizing_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix/polynomial_localizing_matrix.h"
#include "pauli_polynomial_lm_indices.h"

#include "multithreading/multithreading.h"

namespace Moment {
    class RawPolynomial;
}

namespace Moment::Pauli {
    class PauliContext;

    class PauliPolynomialLocalizingMatrix : public PolynomialLocalizingMatrix {
    public:
        const PauliContext& pauli_context;
        PauliPolynomialLMIndex nn_index;

    public:
        PauliPolynomialLocalizingMatrix(const PauliContext& context, SymbolTable& symbols,
                                        const PolynomialFactory& factory,
                                        PauliPolynomialLMIndex index,
                                        PolynomialLocalizingMatrix::ConstituentInfo&& constituents);

        PauliPolynomialLocalizingMatrix(PauliMatrixSystem& system,
                                        NearestNeighbourIndex index,
                                        const std::string& raw_word_name,
                                        PolynomialLocalizingMatrix::ConstituentInfo&& constituents);


    public:
        /** Creates PolynomialLocalizingMatrix from raw polynomial */
        static std::unique_ptr<PauliPolynomialLocalizingMatrix>
        create_from_raw(MaintainsMutex::WriteLock& write_lock, PauliMatrixSystem& system,
                        NearestNeighbourIndex index,  const RawPolynomial& raw_polynomials,
                        Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);
    };
}