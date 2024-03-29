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
#include "symbolic/symbol_errors.h"

#include "utilities/format_factor.h"
#include "utilities/float_utils.h"

#include <cassert>

#include <algorithm>
#include <sstream>

namespace Moment {

    RawPolynomial::RawPolynomial(const Polynomial& symbolic_source, const SymbolTable& symbols) {
        this->data.reserve(symbolic_source.size());
        for (const auto& monomial : symbolic_source) {
            if ((monomial.id < 0) || (monomial.id >= symbols.size())) [[unlikely]] {
                throw errors::unknown_symbol{monomial.id};
            }
            const auto& symbol = symbols[monomial.id];
            if (!symbol.has_sequence()) [[unlikely]] {
                throw std::runtime_error{"An operator sequence was requested for a symbol that does not have one associated with it."};
            }
            this->data.emplace_back(monomial.conjugated ? symbol.sequence_conj() : symbol.sequence(),
                                    monomial.factor);
        }
    }

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

    bool RawPolynomial::is_scalar() const noexcept {
        // Empty sequence is scalar (0)
        if (this->data.empty()) {
            return true;
        }

        // Non-empty sequence is scalar if all of its elements are scalar
        return std::all_of(this->data.begin(), this->data.end(), [](const RawPolynomialElement& os_w) -> bool {
            return os_w.sequence.empty();
        });
    }

    Polynomial RawPolynomial::to_polynomial(const PolynomialFactory& factory) const {
        return factory.construct(*this);
    }

    Polynomial RawPolynomial::to_polynomial_register_symbols(const PolynomialFactory& factory,
                                                             SymbolTable& symbols) const {
        return factory.register_and_construct(symbols, *this);
    }

    RawPolynomial RawPolynomial::add(const RawPolynomial& lhs, const RawPolynomial& rhs, double tolerance) {
        RawPolynomial output{lhs};
        output.data.reserve(lhs.size() + rhs.size());
        std::copy(rhs.data.cbegin(), rhs.data.cend(), std::back_inserter(output.data));
        output.condense(tolerance);
        return output;
    }

    RawPolynomial RawPolynomial::subtract(const RawPolynomial& lhs, const RawPolynomial& rhs, double tolerance) {
        RawPolynomial output{lhs};
        output.data.reserve(lhs.size() + rhs.size());
        std::transform(rhs.data.cbegin(), rhs.data.cend(), std::back_inserter(output.data),
                       [](auto monomial) -> RawPolynomialElement {
            monomial.weight *= -1.0;
            return monomial;
        });
        output.condense(tolerance);
        return output;
    }

    void RawPolynomial::condense(const double tolerance) {
        // Special case: empty vector
        if (this->empty()) {
            return;
        }

        // Special case: one element vector
        if (this->size() == 1) {
            const auto& first = this->data.front();
            if (first.sequence.zero() || approximately_zero(first.weight, tolerance)) {
                this->data.clear();
            }
        }

        // General case: insertion sort and combine into intermediate map
        std::map<hash_t, RawPolynomialElement> intermediate;
        for (auto& element : this->data) {
            if (!element.sequence.zero()) {
                auto [iter_where, did_insert] = intermediate.insert(std::make_pair(element.sequence.hash(), element));
                if (!did_insert) {
                    assert(element.sequence.get_sign() == SequenceSignType::Positive);
                    iter_where->second.weight += element.weight;
                }
            }
        }

        // Move from intermediate map back into data, pruning zeros
        this->data.clear(); // In practice, no need for reserve: container will only shrink!
        for (auto& [key, element] : intermediate) {
            if (!approximately_zero(element.weight, tolerance)) {
                this->data.emplace_back(std::move(element));
            }
        }
    }

}