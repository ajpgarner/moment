/**
 * pauli_moment_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "pauli_moment_matrix.h"

#include <sstream>

namespace Moment::Pauli {
    std::string PauliMomentMatrix::description() const {
        std::stringstream ss;
        ss << "Moment Matrix, Level " << this->Index.moment_matrix_level;
        if (this->Index.neighbours > 0) {
            ss << ", " << this->Index.neighbours << " nearest neighbour";
            if (this->Index.neighbours != 1) {
                ss << "s";
            }
        }
        return ss.str();
    }
}