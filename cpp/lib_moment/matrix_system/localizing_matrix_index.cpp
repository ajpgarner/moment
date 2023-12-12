/**
 * localizing_matrix_index.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "localizing_matrix_index.h"
#include <ostream>

namespace Moment {

    std::ostream& operator<<(std::ostream& os, const LocalizingMatrixIndex& lmi) {
        os << "Localizing matrix for \"" << lmi.Word.formatted_string() << "\" level " << lmi.Level;
        return os;
    }
}