/**
 * pauli_localizing_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix/operator_matrix/localizing_matrix.h"
#include "scenarios/pauli/indices/monomial_index.h"
#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_dictionary.h"

#include "multithreading/multithreading.h"

namespace Moment {
    class SymbolTable;
    class MonomialMatrix;


    namespace Pauli {
        class PauliContext;
        class PauliSequenceGenerator;

        /**
         * Generate 'Pauli' localizing matrix, possibly limited to nearest-neighbours in top row.
         */
        struct PauliLocalizingMatrixGenerator {
        public:
            using Index = Pauli::LocalizingMatrixIndex;
            using OSGIndex = NearestNeighbourIndex;
            const Pauli::LocalizingMatrixIndex index;

             PauliLocalizingMatrixGenerator(const PauliContext& /**/, Pauli::LocalizingMatrixIndex index_in)
                : index{std::move(index_in)} { }

            [[nodiscard]] inline OperatorSequence
            operator()(const OperatorSequence& lhs, const OperatorSequence& rhs) const {
                return lhs * (index.Word * rhs);
            }

            /** Pauli localizing matrices are Hermitian if and only if word is real. */
            [[nodiscard]] inline constexpr static bool
            should_be_hermitian(const Index& index) noexcept {
                return !is_imaginary(index.Word.get_sign());
            }

            /** Localizing matrices will automatically have a prefactor of +1. */
            [[nodiscard]] inline constexpr static std::complex<double>
            determine_prefactor(const Index& /**/) noexcept { return std::complex<double>{1.0, 0.0}; }

            /** Pass-through index to get OSG index. */
            [[nodiscard]] inline constexpr static OSGIndex get_osg_index(const Index& input) {
                return input.Index;
            }

            /** Get nearest-neighbour OSGs */
            [[nodiscard]] inline static const OSGPair& get_generators(const PauliContext& context,
                                                                      const OSGIndex& index) {
                return context.pauli_dictionary().NearestNeighbour(index);
            }

        };
        static_assert(generates_operator_matrices<PauliLocalizingMatrixGenerator,
                                                  Pauli::LocalizingMatrixIndex, PauliContext>);


        /**
         * Localizing matrix composed of Pauli operators.
         */
        class MonomialLocalizingMatrix;
        class MonomialLocalizingMatrix : public OperatorMatrixImpl<Pauli::LocalizingMatrixIndex, PauliContext,
                                                                   PauliLocalizingMatrixGenerator,
                                                                   MonomialLocalizingMatrix> {
        public:
            /**
             * Constructs a moment matrix at the requested hierarchy depth (level) for the supplied context.
             * @param context The setting/scenario.
             * @param level The hierarchy depth.
             * @param mt_policy Whether or not to use multi-threaded creation.
             */
            MonomialLocalizingMatrix(const PauliContext& context, const Pauli::LocalizingMatrixIndex& plmi,
                                     size_t dimension, std::vector<OperatorSequence> op_seq_data)
                 : OperatorMatrixImpl<Pauli::LocalizingMatrixIndex, PauliContext,
                                      PauliLocalizingMatrixGenerator,
                                      MonomialLocalizingMatrix>{context, plmi, dimension, std::move(op_seq_data)}{ }

        };
    }
}