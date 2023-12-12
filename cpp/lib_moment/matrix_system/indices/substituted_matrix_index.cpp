/**
 * substituted_matrix_index.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "substituted_matrix_index.h"

#include "../matrix_system.h"
#include "matrix/symbolic_matrix.h"
#include "symbolic/rules/moment_rulebook.h"

#include <ostream>
#include <sstream>

namespace Moment {

    std::ostream& operator<<(std::ostream& os, SubstitutedMatrixIndex index) {
        // Plain name
        os << "Substituted Matrix: Matrix #" << index.SourceMatrix << ", Rulebook #" << index.Rulebook;
        return os;
    }


    std::string SubstitutedMatrixIndex::to_string(const MatrixSystem& matrix_system) const {

        std::stringstream ss;
        ss << "Substituted Matrix: ";
        // If indexed matrix has name, use it
        const bool matrix_in_range = ((this->SourceMatrix >= 0) && (this->SourceMatrix < matrix_system.size()));
        if (matrix_in_range) {
            ss << matrix_system[this->SourceMatrix].Description();
        } else {
            ss << "Matrix #" << this->SourceMatrix;
        }

        ss << ", Rulebook ";
        // If indexed rulebook has name, use it
        const bool rulebook_in_range = ((this->Rulebook >= 0) && (this->Rulebook < matrix_system.Rulebook.size()));
        if (rulebook_in_range) {
            ss << "\"" << matrix_system.Rulebook.find(this->Rulebook).name() << "\"";
        } else {
            ss << "#" << this->Rulebook;
        }
        return ss.str();
    }
}