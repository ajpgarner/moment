/**
 * pauli_moment_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "nearest_neighbour_matrix.h"
#include "pauli_context.h"
#include "pauli_osg.h"

#include "symbolic/symbol_table.h"
#include "matrix/operator_matrix/operator_matrix_factory.h"

namespace Moment::Pauli {

    std::unique_ptr<MonomialMatrix>
    NearestNeighbourMatrix::create_moment_matrix(const Moment::Pauli::PauliContext& context,
                                                 class Moment::SymbolTable& symbols,
                                                 const Moment::Pauli::NearestNeighbourIndex& nn_index,
                                                 Multithreading::MultiThreadPolicy mt_policy) {
        const auto mm_functor = [](const OperatorSequence& lhs, const OperatorSequence& rhs) {
            return lhs * rhs;
        };

        OperatorMatrixFactory<OperatorMatrix, PauliContext, NearestNeighbourIndex, decltype(mm_functor)>
                creation_context{context, symbols, nn_index, mm_functor, true, mt_policy};

        return creation_context.execute();
    }
}