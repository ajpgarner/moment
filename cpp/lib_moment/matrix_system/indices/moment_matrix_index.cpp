/**
 * moment_matrix_index..cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_matrix_index.h"

#include <sstream>

namespace Moment {
    std::ostream& operator<<(std::ostream& os, Moment::MomentMatrixIndex index) {
        os << "Moment Matrix, Level " << index.Level;
        return os;
    }

    std::string MomentMatrixIndex::to_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

    std::string MomentMatrixIndex::to_string(const MatrixSystem& matrix_system) const {
        return this->to_string();
    }
}