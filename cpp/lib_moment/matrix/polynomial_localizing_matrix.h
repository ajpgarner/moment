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
#include "multithreading/maintains_mutex.h"

#include <complex>
#include <span>
#include <vector>

namespace Moment {

    class MatrixSystem;
    class MonomialMatrix;
    class RawPolynomial;

    class PolynomialLocalizingMatrix : public CompositeMatrix {
    private:
        PolynomialLMIndex index;
        bool aliased_index = false;

    public:
        /** Constructor for non-empty polynomial localizing matrix. */
        PolynomialLocalizingMatrix(const Context& context, SymbolTable& symbols, const PolynomialFactory& factory,
                                   PolynomialLMIndex index, CompositeMatrix::ConstituentInfo&& constituents);

        /** Creates PolynomialLocalizingMatrix from raw polynomial */
        static std::unique_ptr<PolynomialLocalizingMatrix>
        create_from_raw(const MaintainsMutex::WriteLock& write_lock, MatrixSystem& system,
                        size_t level, const RawPolynomial& raw_polynomials,
                        Multithreading::MultiThreadPolicy mt_policy);

    };

}