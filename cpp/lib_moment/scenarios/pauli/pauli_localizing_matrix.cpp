/**
 * pauli_localizing_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "pauli_localizing_matrix.h"
#include "pauli_context.h"
#include "pauli_dictionary.h"
#include "pauli_osg.h"

#include "matrix/operator_matrix/operator_matrix_factory.h"
#include "matrix/monomial_matrix.h"

#include "multithreading/matrix_generation_worker.h"
#include "multithreading/multithreading.h"

#include "scenarios/context.h"

#include <stdexcept>
#include <sstream>
#include <thread>

namespace Moment::Pauli {

    PauliLocalizingMatrix::PauliLocalizingMatrix(const PauliContext& context,
                                                 const PauliLocalizingMatrixIndex& plmi,
                                                 std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat)
            : LocalizingMatrix{context, static_cast<LocalizingMatrixIndex>(plmi), std::move(op_seq_mat)},
              pauliContext{context}, PauliIndex{plmi} {

    }

    PauliLocalizingMatrix::PauliLocalizingMatrix(PauliLocalizingMatrix&& src) noexcept = default;

    PauliLocalizingMatrix::~PauliLocalizingMatrix() noexcept = default;

    const OSGPair& PauliLocalizingMatrix::generators() const {
        const auto& dictionary = dynamic_cast<const PauliDictionary&>(context.dictionary());
        return dictionary.NearestNeighbour(this->PauliIndex.Index);
    }

    std::string PauliLocalizingMatrix::description() const {
        const auto& nn_info = this->PauliIndex.Index;
        std::stringstream ss;
        ss << "Localizing Matrix, Level " << nn_info.moment_matrix_level;
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
    PauliLocalizingMatrix::create_matrix(const PauliContext& context, class SymbolTable& symbols,
                                     const PauliLocalizingMatrixIndex& plmi,
                                     Multithreading::MultiThreadPolicy mt_policy) {

        // Check word sign type
        const bool should_be_hermitian = !is_imaginary(plmi.Word.get_sign());

        // Check if symmetrized
        const bool has_aliases = context.can_have_aliases();

        if (has_aliases) {
            // Define localizing matrix element functor with aliases
            const auto lm_functor = [&context, &plmi](const OperatorSequence& lhs, const OperatorSequence& rhs) {
                return context.simplify_as_moment(lhs * (plmi.Word * rhs));
            };

            // Do creation
            OperatorMatrixFactory<PauliLocalizingMatrix, class PauliContext, NearestNeighbourIndex, decltype(lm_functor)>
                    creation_context{context, symbols, plmi.Index, lm_functor, should_be_hermitian, mt_policy};
            return creation_context.execute(plmi);
        } else {

            // Define localizing matrix element functor
            const auto lm_functor = [&plmi](const OperatorSequence& lhs, const OperatorSequence& rhs) {
                return lhs * (plmi.Word * rhs);
            };

            // Do creation
            OperatorMatrixFactory<PauliLocalizingMatrix, class PauliContext, NearestNeighbourIndex, decltype(lm_functor)>
                    creation_context{context, symbols, plmi.Index, lm_functor, should_be_hermitian, mt_policy};
            return creation_context.execute(plmi);
        }
    }

}