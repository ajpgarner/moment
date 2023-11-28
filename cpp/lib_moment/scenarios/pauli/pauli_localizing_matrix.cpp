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
    std::string PauliLocalizingMatrix::description() const {
        const auto& nn_info = this->Index.Index;
        std::stringstream ss;
        ss << "Localizing Matrix, Level " << nn_info.moment_matrix_level;
        if (nn_info.neighbours > 0) {
            ss << ", " << nn_info.neighbours << " nearest neighbour";
            if (nn_info.neighbours != 1) {
                ss << "s";
            }
        }
        ss << ", Word " << this->Index.Word;
        return ss.str();
    }

}