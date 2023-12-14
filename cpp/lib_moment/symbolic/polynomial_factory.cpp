/**
 * polynomial_factory.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_factory.h"

#include "symbol_table.h"

#include "dictionary/raw_polynomial.h"

#include <algorithm>
#include <iostream>
#include <numeric>


namespace Moment {
    std::ostream& operator<<(std::ostream& os, const PolynomialFactory& factory) {
        os << factory.name() << ", floating-point tolerance multiplier = " << factory.zero_tolerance << ".";
        return os;
    }


    SmallVector<size_t, 1> PolynomialFactory::presort_data(Polynomial::storage_t& data) const {
        // Fill indices
        SmallVector<size_t, 1> sort_order(data.size(), 0);
        std::iota(sort_order.begin(), sort_order.end(), 0);

        // Get sort order
        std::stable_sort(sort_order.begin(), sort_order.end(),
                         [&](const size_t lhs_index, size_t rhs_index) {
                             return this->less(data[lhs_index], data[rhs_index]);
                         });

        // Apply sort, if done
        if (!std::is_sorted(sort_order.begin(), sort_order.end())) {
            Polynomial::storage_t sorted_data;
            sorted_data.reserve(data.size());
            for (const auto idx : sort_order) {
                sorted_data.emplace_back(data[idx]);
            }
            data.swap(sorted_data);
        }
        return sort_order;
    }

    namespace {
        [[nodiscard]] inline Polynomial::storage_t
        make_storage_data_from_raw(const SymbolTable& symbols, const RawPolynomial& raw) {
            Polynomial::storage_t output_storage;
            output_storage.reserve(raw.size());
            for (const auto& elem : raw) {
                auto search = symbols.where(elem.sequence);
                if (!search.found()) [[unlikely]] {
                    throw errors::unregistered_operator_sequence{elem.sequence.formatted_string(), elem.sequence.hash()};
                }
                assert(search.symbol != nullptr); // ^- above should throw if this is true.
                output_storage.emplace_back(search->Id(), elem.weight, search.is_conjugated);
            }
            return output_storage;
        }

        [[nodiscard]] inline Polynomial::storage_t
        register_and_make_storage_data_from_raw(SymbolTable& symbols, const RawPolynomial& raw) {
            Polynomial::storage_t output_storage;
            output_storage.reserve(raw.size());
            for (const auto& elem : raw) {
                auto search = symbols.where(elem.sequence);
                if (!search.found()) {
                    // Otherwise, copy symbol into symbol table, and get its inserted ID
                    const symbol_name_t inserted_id = symbols.merge_in(OperatorSequence{elem.sequence});
                    auto search_again = symbols.where(elem.sequence); // Search again, could be alised, or conjugated.
                    assert(search_again.found());
                    output_storage.emplace_back(search_again->Id(), elem.weight, search_again.is_conjugated);
                } else {
                    assert(search.symbol != nullptr);
                    output_storage.emplace_back(search->Id(), elem.weight, search.is_conjugated);
                }
            }
            return output_storage;
        }
    }

    Polynomial PolynomialFactory::construct(const RawPolynomial& raw) const {
        return (*this)(make_storage_data_from_raw(this->symbols, raw));
    }

    Polynomial PolynomialFactory::register_and_construct(SymbolTable& write_symbols,
                                                         const RawPolynomial& raw) const {
        assert(&(this->symbols) == &write_symbols); // Write symbol table must match factory.
        return (*this)(register_and_make_storage_data_from_raw(write_symbols, raw));
    }

    Polynomial PolynomialFactory::sum(const Monomial& lhs, const Monomial& rhs) const {
        // "Monomial"-like sum
        if ((lhs.id == rhs.id) && (lhs.conjugated == rhs.conjugated)) {
            std::complex<double> factor = lhs.factor + rhs.factor;
            if (approximately_zero(factor, this->zero_tolerance)) {
                return Polynomial::Zero();
            } else {
                return Polynomial{Monomial{lhs.id, factor, lhs.conjugated}};
            }
        }

        // Otherwise, construct as two element sum:
        if (this->less(lhs, rhs)) {
            return Polynomial{Polynomial::init_raw_tag{}, Polynomial::storage_t{lhs, rhs}};
        } else {
            return Polynomial{Polynomial::init_raw_tag{}, Polynomial::storage_t{rhs, lhs}};
        }
    }

    Polynomial PolynomialFactory::sum(const Polynomial& lhs, const Monomial& rhs) const {
        // TODO: Efficient addition assuming LHS is sorted
        Polynomial output{lhs};
        this->append(output, Polynomial(rhs, this->zero_tolerance)); // <- virtual call.
        return output;
    }

    Polynomial PolynomialFactory::sum(const Polynomial& lhs, const Polynomial& rhs) const {
        // TODO: Efficient addition assuming LHS and RHS are sorted
        Polynomial output{lhs};
        this->append(output, rhs); // <- virtual call.
        return output;
    }

    size_t PolynomialFactory::maximum_degree(const Polynomial& poly) const {
        size_t largest_monomial = 0;
        for (auto& mono : poly) {
            if (mono.id > 1) {
                assert(mono.id < this->symbols.size());
                const auto& symbol = this->symbols[mono.id];
                if (symbol.has_sequence()) [[likely]] {
                    largest_monomial = std::max(symbol.sequence().size(), largest_monomial);
                }
            }
        }
        return largest_monomial;
    }




}