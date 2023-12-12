/**
 * polynomial_commutator_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "commutator_matrix.h"
#include "pauli_polynomial_lm_indices.h"

#include "matrix/composite_matrix.h"
#include "matrix/polynomial_localizing_matrix.h"

#include "multithreading/maintains_mutex.h"
#include "multithreading/multithreading.h"

namespace Moment {
    class RawPolynomial;
}

namespace Moment::Pauli {

    class PauliContext;

    using PolynomialCommutatorMatrixIndex = PauliPolynomialLMIndex;

    class PolynomialCommutatorMatrix : public CompositeMatrix {
    public:
        const PauliContext& pauli_context;
        const PolynomialCommutatorMatrixIndex index;

    public:
        PolynomialCommutatorMatrix(const PauliContext& context, SymbolTable& symbols,
                                   const PolynomialFactory& factory,
                                   PolynomialCommutatorMatrixIndex index,
                                   CompositeMatrix::ConstituentInfo&& constituents);

        PolynomialCommutatorMatrix(PauliMatrixSystem& system, NearestNeighbourIndex index,
                                   const std::string& raw_word_name, CompositeMatrix::ConstituentInfo&& constituents);

        /** Creates PolynomialLocalizingMatrix from raw polynomial */
        static std::unique_ptr<PolynomialCommutatorMatrix>
        create_from_raw(MaintainsMutex::WriteLock& write_lock, PauliMatrixSystem& system,
                        NearestNeighbourIndex index,  const RawPolynomial& raw_polynomials,
                        Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);
    };

    class PolynomialAnticommutatorMatrix : public CompositeMatrix {
    protected:
    public:
        const PauliContext& pauli_context;
        const PolynomialCommutatorMatrixIndex index;

    public:
        PolynomialAnticommutatorMatrix(const PauliContext& context, SymbolTable& symbols,
                                       const PolynomialFactory& factory,
                                       PolynomialCommutatorMatrixIndex index,
                                       CompositeMatrix::ConstituentInfo&& constituents);


        PolynomialAnticommutatorMatrix(PauliMatrixSystem& system, NearestNeighbourIndex index,
                                       const std::string& raw_word_name,
                                       CompositeMatrix::ConstituentInfo&& constituents);

        /** Creates PolynomialLocalizingMatrix from raw polynomial */
        static std::unique_ptr<PolynomialAnticommutatorMatrix>
        create_from_raw(MaintainsMutex::WriteLock& write_lock, PauliMatrixSystem& system,
                        NearestNeighbourIndex index, const RawPolynomial& raw_polynomials,
                        Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);
    };

    /**
      * Factory to make polynomial localizing matrices restricted to nearest neighbours.
      */
    class PolynomialCommutatorMatrixFactory {
    public:
        using Index = PolynomialCommutatorMatrixIndex;

    private:
        PauliMatrixSystem& system;

    public:
        explicit PolynomialCommutatorMatrixFactory(PauliMatrixSystem& system) : system{system} {}

        explicit PolynomialCommutatorMatrixFactory(MatrixSystem& system);

        [[nodiscard]] std::pair<ptrdiff_t, PolynomialMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, const Index& index,
                   Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                    ptrdiff_t offset, PolynomialMatrix& matrix);
    };


    using PolynomialCommutatorMatrixIndices = MatrixIndices<PolynomialMatrix, PolynomialCommutatorMatrixIndex,
                                                            PauliPolynomialLMIndexStorage,
                                                            PolynomialCommutatorMatrixFactory,
                                                             PauliMatrixSystem>;


    /**
      * Factory to make polynomial localizing matrices restricted to nearest neighbours.
      */
    class PolynomialAnticommutatorMatrixFactory {
    public:
        using Index = PolynomialCommutatorMatrixIndex;

    private:
        PauliMatrixSystem& system;

    public:
        explicit PolynomialAnticommutatorMatrixFactory(PauliMatrixSystem& system) : system{system} {}

        explicit PolynomialAnticommutatorMatrixFactory(MatrixSystem& system);

        [[nodiscard]] std::pair<ptrdiff_t, PolynomialMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, const Index& index,
                   Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                    ptrdiff_t offset, PolynomialMatrix& matrix);
    };


    using PolynomialAnticommutatorMatrixIndices = MatrixIndices<PolynomialMatrix, PolynomialCommutatorMatrixIndex,
                                                                PauliPolynomialLMIndexStorage,
                                                                PolynomialAnticommutatorMatrixFactory,
                                                                PauliMatrixSystem>;

}