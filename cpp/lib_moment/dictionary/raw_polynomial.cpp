/**
 * raw_polynomial.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "raw_polynomial.h"

#include "scenarios/contextual_os.h"

#include "symbolic/polynomial.h"
#include "symbolic/polynomial_factory.h"
#include "symbolic/symbol_table.h"

#include "utilities/format_factor.h"
#include "utilities/float_utils.h"

#include <sstream>

namespace Moment {

    std::string RawPolynomial::to_string(const Context& context) const {
        std::stringstream ss;
        ContextualOS cSS{ss, context};
        cSS.format_info.show_braces = true;

        bool done_once = false;
        for (const auto& elem : this->data) {
            format_factor(cSS, elem.weight, false, done_once);
            cSS << elem.sequence;
            done_once = true;
        }

        return ss.str();
    }

    Polynomial RawPolynomial::to_polynomial(const PolynomialFactory& factory) const {
        return factory.construct(*this);
    }

    Polynomial RawPolynomial::to_polynomial_register_symbols(const PolynomialFactory& factory,
                                                             SymbolTable& symbols) const {
        return factory.register_and_construct(symbols, *this);
    }
}