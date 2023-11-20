/**
 * pauli_moment_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */


#include "pauli_moment_matrix.h"
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

    PauliMomentMatrix::PauliMomentMatrix(const PauliContext& context, const NearestNeighbourIndex& nn_index,
                               std::unique_ptr<OperatorMatrix::OpSeqMatrix> op_seq_mat)
            : MomentMatrix{context, nn_index.moment_matrix_level, std::move(op_seq_mat)},
              pauliContext{context}, NNIndex{nn_index} {

    }

    PauliMomentMatrix::PauliMomentMatrix(PauliMomentMatrix&& src) noexcept = default;

    PauliMomentMatrix::~PauliMomentMatrix() noexcept = default;

    const OSGPair& PauliMomentMatrix::generators() const {
        const auto& dictionary = dynamic_cast<const PauliDictionary&>(context.dictionary());
        return dictionary.NearestNeighbour(this->NNIndex);
    }

    std::string PauliMomentMatrix::description() const {
        std::stringstream ss;
        ss << "Moment Matrix, Level " << this->NNIndex.moment_matrix_level;
        if (this->NNIndex.neighbours > 0) {
            ss << ", " << this->NNIndex.neighbours << " nearest neighbour";
            if (this->NNIndex.neighbours != 1) {
                ss << "s";
            }
            if (this->NNIndex.wrapped) {
                ss << " (wrapped)";
            }
        }
        return ss.str();
    }


    std::unique_ptr<MonomialMatrix>
    PauliMomentMatrix::create_matrix(const PauliContext& context, class SymbolTable& symbols,
                                     const NearestNeighbourIndex& nn_index,
                                     Multithreading::MultiThreadPolicy mt_policy) {
        const auto mm_functor = [](const OperatorSequence& lhs, const OperatorSequence& rhs) {
            return lhs * rhs;
        };

        OperatorMatrixFactory<PauliMomentMatrix, PauliContext, NearestNeighbourIndex, decltype(mm_functor)>
                creation_context{context, symbols, nn_index, mm_functor, true, mt_policy};

        return creation_context.execute(nn_index);
    }

}