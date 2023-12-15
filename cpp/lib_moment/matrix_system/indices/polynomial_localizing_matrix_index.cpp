/**
 * polynomial_localizing_matrix_index.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "polynomial_localizing_matrix_index.h"

#include "../matrix_system.h"

#include <sstream>

namespace Moment {

    std::ostream& operator<<(std::ostream& os, const PolynomialLocalizingMatrixIndex& plmi) {
        os << "Localizing Matrix, Level " << plmi.Level << ", Polynomial Word";
        return os;
    }

    [[nodiscard]] std::string PolynomialLocalizingMatrixIndex::to_string(const Context& context,
                                                                         const SymbolTable& symbols) const {
        std::stringstream ss;

        ContextualOS cSS{ss, context, symbols};
        cSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;
        cSS.format_info.show_braces = false;
        cSS << "Localizing Matrix, Level " << this->Level << ", Word \"" << this->Polynomial << "\"";

        return ss.str();
    }

    std::string PolynomialLocalizingMatrixIndex::to_string(const MatrixSystem& matrix_system) const {
        return this->to_string(matrix_system.Context(), matrix_system.Symbols());
    }

    std::string PolynomialLocalizingMatrixIndex::raw_to_string(const Context& context, const SymbolTable& symbols,
                                                           size_t level,
                                                           const RawPolynomial& raw) {
        std::stringstream ss;

        ContextualOS cSS{ss, context, symbols};
        cSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;
        cSS.format_info.show_braces = false;
        cSS << "Localizing Matrix, Level " << level << ", Word \"" << raw.to_string(context) << "\"";
        return ss.str();
    }
}
