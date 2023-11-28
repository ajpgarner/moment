/**
 * commutator_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "pauli_localizing_matrix_indices.h"
#include "matrix/operator_matrix/operator_matrix.h"

namespace Moment::Pauli {
    class PauliContext;

    using CommutatorMatrixIndex = PauliLocalizingMatrixIndex;

    class MonomialAnticommutatorMatrix : public OperatorMatrix {
    public:
        /** Context */
        const PauliContext& pauliContext;

        /** The hierarchy level and polynomial defining this matrix */
        const PauliLocalizingMatrixIndex PauliIndex;

    public:
        /**
         * Constructs a moment matrix at the requested hierarchy depth (level) for the supplied context.
         * @param context The setting/scenario.
         * @param level The hierarchy depth.
         * @param mt_policy Whether or not to use multi-threaded creation.
         */
        MonomialAnticommutatorMatrix(const PauliContext& context, const PauliLocalizingMatrixIndex& plmi,
                              std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat);

        MonomialAnticommutatorMatrix(const MonomialAnticommutatorMatrix&) = delete;

        MonomialAnticommutatorMatrix(MonomialAnticommutatorMatrix&& src) noexcept;

        ~MonomialAnticommutatorMatrix() noexcept;

        /**
         * Get nearest-neighbour info from index.
         */
        [[nodiscard]] const NearestNeighbourIndex& NearestNeighbour() const noexcept {
            return this->PauliIndex.Index;
        }

        /**
         * The generators associated with this matrix.
         */
        [[nodiscard]] const OSGPair& generators() const override;

        /**
         * Description of the localizing matrix.
         */
        [[nodiscard]] std::string description() const override;

    public:

        /**
         * Full creation stack, with possible multithreading.
         */
        [[nodiscard]] static std::unique_ptr<MonomialMatrix>
        create_matrix(const PauliContext& context, class SymbolTable& symbols,
                      const PauliLocalizingMatrixIndex& plmi,
                      Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);

    };
}