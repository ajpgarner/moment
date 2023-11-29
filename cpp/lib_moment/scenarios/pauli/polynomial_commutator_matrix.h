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

        [[nodiscard]] std::string not_found_msg(const Index& pmi) const;

        [[nodiscard]] std::unique_lock<std::shared_mutex> get_write_lock();
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

        [[nodiscard]] std::string not_found_msg(const Index& pmi) const;

        [[nodiscard]] std::unique_lock<std::shared_mutex> get_write_lock();
    };


    using PolynomialAnticommutatorMatrixIndices = MatrixIndices<PolynomialMatrix, PolynomialCommutatorMatrixIndex,
                                                                PauliPolynomialLMIndexStorage,
                                                                PolynomialAnticommutatorMatrixFactory,
                                                                PauliMatrixSystem>;

}