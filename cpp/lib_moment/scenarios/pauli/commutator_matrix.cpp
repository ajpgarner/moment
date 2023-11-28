/**
 * commutator_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "commutator_matrix.h"

namespace Moment::Pauli {

    std::string MonomialCommutatorMatrix::description() const {
        const auto& nn_info = this->Index.Index;
        std::stringstream ss;
        ss << "Commutator matrix, level " << nn_info.moment_matrix_level;
        if (nn_info.neighbours > 0) {
            ss << ", " << nn_info.neighbours << " nearest neighbour";
            if (nn_info.neighbours != 1) {
                ss << "s";
            }
        }
        ss << ", Word " << this->Index.Word;
        return ss.str();
    }

    std::string MonomialAnticommutatorMatrix::description() const {
        const auto& nn_info = this->Index.Index;
        std::stringstream ss;
        ss << "Anti-commutator matrix, level " << nn_info.moment_matrix_level;
        if (nn_info.neighbours > 0) {
            ss << ", " << nn_info.neighbours << " nearest neighbour";
            if (nn_info.neighbours != 1) {
                ss << "s";
            }
        }
        ss << ", Word " << this->Index.Word;
        return ss.str();
    }

//
//
//    std::unique_ptr<MonomialMatrix>
//    MonomialAnticommutatorMatrix::create_matrix(const PauliContext& context, class SymbolTable& symbols,
//                                         const PauliLocalizingMatrixIndex& plmi,
//                                         Multithreading::MultiThreadPolicy mt_policy) {
//
//        // Check word sign type
//        const bool should_be_hermitian = !is_imaginary(plmi.Word.get_sign());
//
//        // Check if symmetrized
//        const bool has_aliases = context.can_have_aliases();
//
//        if (has_aliases) {
//            // Define localizing matrix element functor with aliases
//            const auto lm_functor = [&context, &plmi](const OperatorSequence& lhs, const OperatorSequence& rhs) {
//                return context.simplify_as_moment(context.anticommutator(lhs * rhs, plmi.Word));
//            };
//
//            // Do creation
//            OperatorMatrixFactory<MonomialAnticommutatorMatrix, class PauliContext,
//                                  NearestNeighbourIndex, decltype(lm_functor)>
//                    creation_context{context, symbols, plmi.Index, lm_functor, should_be_hermitian, 2.0, mt_policy};
//            return creation_context.execute(plmi);
//        } else {
//
//            // Define localizing matrix element functor
//            const auto lm_functor = [&context, &plmi](const OperatorSequence& lhs, const OperatorSequence& rhs) {
//                return context.anticommutator(lhs * rhs, plmi.Word);
//            };
//
//            // Do creation
//            OperatorMatrixFactory<MonomialAnticommutatorMatrix, class PauliContext,
//                                  NearestNeighbourIndex, decltype(lm_functor)>
//                    creation_context{context, symbols, plmi.Index, lm_functor, should_be_hermitian, 2.0, mt_policy};
//            return creation_context.execute(plmi);
//        }
//    }
}