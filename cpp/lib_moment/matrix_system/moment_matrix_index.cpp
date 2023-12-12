/**
 * moment_matrix_index..cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_matrix_index.h"
#include <ostream>

namespace Moment {
    std::ostream& operator<<(std::ostream& os, Moment::MomentMatrixIndex index) {
        os << "Moment matrix level " << index.Level;
        return os;
    }
}