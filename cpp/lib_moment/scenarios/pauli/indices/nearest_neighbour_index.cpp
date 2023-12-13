/**
 * nearest_neighbour_index.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "nearest_neighbour_index.h"

#include <sstream>

namespace Moment::Pauli {

    std::string MomentMatrixIndex::to_string() const {
        std::stringstream ss;
        ss << "Moment Matrix, Level " << this->moment_matrix_level;
        if (this->neighbours > 0) {
            ss << ", " << this->neighbours << " Nearest Neighbour";
            if (this->neighbours != 1) {
                ss << "s";
            }
        }
        return ss.str();
    }
}