/**
 * localizing_matrix_index.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "localizing_matrix_index.h"
#include "../matrix_system.h"
#include "scenarios/context.h"

#include <sstream>

namespace Moment {

    std::ostream& operator<<(std::ostream& os, const LocalizingMatrixIndex& lmi) {
        os << "Localizing matrix, Level " << lmi.Level << ", Word \"" << lmi.Word.formatted_string() << "\"";
        return os;
    }

    std::string LocalizingMatrixIndex::to_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

    std::string LocalizingMatrixIndex::to_string(const Context& context) const {
        std::stringstream ss;
        ss << "Localizing Matrix, Level " << this->Level << ", Word \""
            << context.format_sequence(this->Word) << "\"";
        return ss.str();
    }

    std::string LocalizingMatrixIndex::to_string(const MatrixSystem& matrix_system) const {
        return this->to_string(matrix_system.Context());
    }
}