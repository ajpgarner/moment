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

    Polynomial RawPolynomial::to_polynomial(const PolynomialFactory& factory, const SymbolTable& symbols) const {
        Polynomial::storage_t output_storage;
        output_storage.reserve(this->data.size());
        for (const auto& elem : this->data) {
            auto search = symbols.where(elem.sequence);
            if (!search.found()) [[unlikely]] {
                throw std::runtime_error{
                    "RawPolynomial contained at least one operator sequence that has not yet been registered in the symbol table!"
                };
            }
            assert(search.symbol != nullptr); // ^- above should throw if this is true.
            output_storage.emplace_back(search->Id(), elem.weight, search.is_conjugated);
        }
        return factory(std::move(output_storage));
    }
}