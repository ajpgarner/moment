/**
 * pauli_moment_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "multithreading/multithreading.h"
#include "matrix/monomial_matrix.h"

#include "nearest_neighbour_index.h"

#include <memory>

namespace Moment {
    class SymbolTable;


    namespace Pauli {
        class PauliContext;

        /**
         * Scalar extensions of monomial moment matrix
         */
        class NearestNeighbourMatrix {
        public:

            /**
             * Full creation stack, with possible multithreading.
             */
            [[nodiscard]] static std::unique_ptr<MonomialMatrix>
            create_moment_matrix(const PauliContext& context, class SymbolTable& symbols,
                 const NearestNeighbourIndex& nn_index,
                 Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);


        };

    }
}