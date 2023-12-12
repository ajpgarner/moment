/**
 * substituted_matrix_index.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "substituted_matrix_index.h"

#include <ostream>

namespace Moment {

    std::ostream& operator<<(std::ostream& os, SubstitutedMatrixIndex index) {
        // Plain name
        os << "Substituted matrix from matrix " << index.SourceMatrix << " with rulebook " << index.Rulebook;
        return os;
    }
}