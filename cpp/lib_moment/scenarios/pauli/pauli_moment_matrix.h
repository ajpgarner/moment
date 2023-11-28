/**
 * moment_matrix.h
 *
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "matrix/operator_matrix/moment_matrix.h"
#include "nearest_neighbour_index.h"
#include "pauli_context.h"
#include "pauli_dictionary.h"

#include "multithreading/multithreading.h"

namespace Moment {
    class SymbolTable;
    class MonomialMatrix;

    namespace Pauli {
        class PauliContext;

        /**
         * Generate 'Pauli' moment matrix, possibly limited to nearest-neighbours in top row.
         */
        struct PauliMomentMatrixGenerator {
        public:
            using OSGIndex = NearestNeighbourIndex;

            const NearestNeighbourIndex index;

            constexpr PauliMomentMatrixGenerator(const PauliContext& /**/, NearestNeighbourIndex index_in)
                    : index{std::move(index_in)} { }

            [[nodiscard]] inline OperatorSequence
            operator()(const OperatorSequence& lhs, const OperatorSequence& rhs) const {
                return lhs * rhs;
            }

            /** Moment matrices are always Hermitian. */
            [[nodiscard]] inline constexpr static bool
            should_be_hermitian(const NearestNeighbourIndex& /**/) noexcept { return true; }

            /** Moment matrices always have a prefactor of +1. */
            [[nodiscard]] inline constexpr static std::complex<double>
            determine_prefactor(const NearestNeighbourIndex& /**/) noexcept { return std::complex<double>{1.0, 0.0}; }

            /** Pass-through index to get OSG index. */
            [[nodiscard]] inline constexpr static OSGIndex get_osg_index(const NearestNeighbourIndex& input) {
                return input;
            }

            /** Get nearest-neighbour OSGs */
            [[nodiscard]] inline static const OSGPair& get_generators(const PauliContext& context,
                                                                      const NearestNeighbourIndex& index) {
                return context.pauli_dictionary().NearestNeighbour(index);
            }

        };
        static_assert(generates_operator_matrices<PauliMomentMatrixGenerator, NearestNeighbourIndex, PauliContext>);


        /**
         * Moment matrix of Pauli operators.
         */
        class PauliMomentMatrix;
        class PauliMomentMatrix
                : public OperatorMatrixImpl<NearestNeighbourIndex, PauliContext, PauliMomentMatrixGenerator,
                                            PauliMomentMatrix> {
        public:
            /**
             * Constructs a moment matrix at the requested hierarchy depth (level) for the supplied context.
             * @param context The setting/scenario.
             * @param level The hierarchy depth.
             * @param mt_policy Whether or not to use multi-threaded creation.
             */
            PauliMomentMatrix(const PauliContext& context, const NearestNeighbourIndex& level,
                              std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat)
                      : OperatorMatrixImpl<NearestNeighbourIndex, PauliContext, PauliMomentMatrixGenerator,
                    PauliMomentMatrix>{context, level, std::move(op_seq_mat)} { }


            [[nodiscard]] std::string description() const override;


        };
    }
}