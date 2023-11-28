/**
 * commutator_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "commutator_matrix.h"

#include "pauli_context.h"
#include "pauli_dictionary.h"

#include "matrix/operator_matrix/operator_matrix_factory.h"

namespace Moment::Pauli {

    MonomialAnticommutatorMatrix::MonomialAnticommutatorMatrix(const PauliContext& context,
                                                 const PauliLocalizingMatrixIndex& plmi,
                                                 std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat)
            : OperatorMatrix{context, std::move(op_seq_mat)},
              pauliContext{context}, PauliIndex{plmi} {

    }

    MonomialAnticommutatorMatrix::MonomialAnticommutatorMatrix(MonomialAnticommutatorMatrix&& src) noexcept = default;

    MonomialAnticommutatorMatrix::~MonomialAnticommutatorMatrix() noexcept = default;

    const OSGPair& MonomialAnticommutatorMatrix::generators() const {
        const auto& dictionary = dynamic_cast<const PauliDictionary&>(context.dictionary());
        return dictionary.NearestNeighbour(this->PauliIndex.Index);
    }

    std::string MonomialAnticommutatorMatrix::description() const {
        const auto& nn_info = this->PauliIndex.Index;
        std::stringstream ss;
        ss << "Anti-commutator matrix, level " << nn_info.moment_matrix_level;
        if (nn_info.neighbours > 0) {
            ss << ", " << nn_info.neighbours << " nearest neighbour";
            if (nn_info.neighbours != 1) {
                ss << "s";
            }
        }
        ss << ", Word " << this->PauliIndex.Word;
        return ss.str();
    }


    std::unique_ptr<MonomialMatrix>
    MonomialAnticommutatorMatrix::create_matrix(const PauliContext& context, class SymbolTable& symbols,
                                         const PauliLocalizingMatrixIndex& plmi,
                                         Multithreading::MultiThreadPolicy mt_policy) {

        // Check word sign type
        const bool should_be_hermitian = !is_imaginary(plmi.Word.get_sign());

        // Check if symmetrized
        const bool has_aliases = context.can_have_aliases();

        if (has_aliases) {
            // Define localizing matrix element functor with aliases
            const auto lm_functor = [&context, &plmi](const OperatorSequence& lhs, const OperatorSequence& rhs) {
                return context.simplify_as_moment(context.anticommutator(lhs * rhs, plmi.Word));
            };

            // Do creation
            OperatorMatrixFactory<MonomialAnticommutatorMatrix, class PauliContext,
                                  NearestNeighbourIndex, decltype(lm_functor)>
                    creation_context{context, symbols, plmi.Index, lm_functor, should_be_hermitian, 2.0, mt_policy};
            return creation_context.execute(plmi);
        } else {

            // Define localizing matrix element functor
            const auto lm_functor = [&context, &plmi](const OperatorSequence& lhs, const OperatorSequence& rhs) {
                return context.anticommutator(lhs * rhs, plmi.Word);
            };

            // Do creation
            OperatorMatrixFactory<MonomialAnticommutatorMatrix, class PauliContext,
                                  NearestNeighbourIndex, decltype(lm_functor)>
                    creation_context{context, symbols, plmi.Index, lm_functor, should_be_hermitian, 2.0, mt_policy};
            return creation_context.execute(plmi);
        }
    }
}