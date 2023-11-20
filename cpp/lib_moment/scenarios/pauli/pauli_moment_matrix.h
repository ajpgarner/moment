/**
 * moment_matrix.h
 *
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "matrix/operator_matrix/moment_matrix.h"
#include "nearest_neighbour_index.h"

#include "multithreading/multithreading.h"

namespace Moment {
    class SymbolTable;
    class MonomialMatrix;

    namespace Pauli {
        class PauliContext;

        class PauliSequenceGenerator;

        /**
         * MomentMatrix, of operators.
         */
        class PauliMomentMatrix : public MomentMatrix {
        public:
            /** Context */
            const PauliContext& pauliContext;

            /** The Level of moment matrix defined */
            const NearestNeighbourIndex index;

        public:
            /**
             * Constructs a moment matrix at the requested hierarchy depth (level) for the supplied context.
             * @param context The setting/scenario.
             * @param level The hierarchy depth.
             * @param mt_policy Whether or not to use multi-threaded creation.
             */
            PauliMomentMatrix(const PauliContext& context, const NearestNeighbourIndex& level,
                              std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat);

            PauliMomentMatrix(const PauliMomentMatrix&) = delete;

            PauliMomentMatrix(PauliMomentMatrix&& src) noexcept;

            ~PauliMomentMatrix() noexcept;

            /**
             * The generators associated with this matrix
             */
            [[nodiscard]] const OSGPair& generators() const override;

            [[nodiscard]] std::string description() const override;

        public:

            /**
             * Full creation stack, with possible multithreading.
             */
            [[nodiscard]] static std::unique_ptr<MonomialMatrix>
            create_matrix(const PauliContext& context, class SymbolTable& symbols,
                 const NearestNeighbourIndex& nn_index,
                 Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);


        };
    }
}