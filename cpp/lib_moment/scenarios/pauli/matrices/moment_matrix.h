/**
 * moment_matrix.h
 *
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "matrix/operator_matrix/moment_matrix.h"

#include "scenarios/pauli/indices/nearest_neighbour_index.h"
#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_dictionary.h"

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
            using Index = Pauli::MomentMatrixIndex;
            using OSGIndex = NearestNeighbourIndex;

            const Pauli::MomentMatrixIndex index;

            constexpr PauliMomentMatrixGenerator(const PauliContext& /**/,  Index index_in)
                    : index{std::move(index_in)} { }

            [[nodiscard]] inline OperatorSequence
            operator()(const OperatorSequence& lhs, const OperatorSequence& rhs) const {
                return lhs * rhs;
            }

            /** Moment matrices are always Hermitian. */
            [[nodiscard]] inline constexpr static bool
            should_be_hermitian(const Index& /**/) noexcept { return true; }

            /** Moment matrices always have a prefactor of +1. */
            [[nodiscard]] inline constexpr static std::complex<double>
            determine_prefactor(const Index& /**/) noexcept { return std::complex<double>{1.0, 0.0}; }

            /** Pass-through index to get OSG index. */
            [[nodiscard]] inline constexpr static OSGIndex get_osg_index(const Index& input) {
                static_assert(std::derived_from<Index, OSGIndex>);
                return static_cast<OSGIndex>(input);
            }

            /** Get nearest-neighbour OSGs */
            [[nodiscard]] inline static const OSGPair& get_generators(const PauliContext& context,
                                                                      const NearestNeighbourIndex& index) {
                return context.pauli_dictionary().NearestNeighbour(index);
            }

        };
        static_assert(generates_operator_matrices<PauliMomentMatrixGenerator, Pauli::MomentMatrixIndex, PauliContext>);


        /**
         * Moment matrix of Pauli operators.
         */
        class MomentMatrix;
        class MomentMatrix
                : public OperatorMatrixImpl<Pauli::MomentMatrixIndex, PauliContext, PauliMomentMatrixGenerator,
                                            Pauli::MomentMatrix> {
        public:
            /**
             * Constructs a moment matrix at the requested hierarchy depth (level) for the supplied context.
             * @param context The setting/scenario.
             * @param level The hierarchy depth.
             * @param mt_policy Whether or not to use multi-threaded creation.
             */
            MomentMatrix(const PauliContext& context, const Pauli::MomentMatrixIndex& index,
                         size_t dimension, std::vector<OperatorSequence> op_seq_data)
                      : OperatorMatrixImpl<Pauli::MomentMatrixIndex, PauliContext, PauliMomentMatrixGenerator,
                                           Pauli::MomentMatrix>{context, index, dimension, std::move(op_seq_data)} { }
        };
    }
}