/**
 * symbol_combo.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "symbol_combo.h"
#include "symbol_table.h"

#include "utilities/float_utils.h"

#include <iostream>
#include <sstream>

namespace Moment {

    namespace {

        struct LexEqComparator {
            bool operator()(const Monomial &lhs, const Monomial &rhs) const noexcept {
                if (lhs.id != rhs.id) {
                    return false;
                }
                if (lhs.conjugated != rhs.conjugated) {
                    return false;
                }
                return true;
            }
        };
    }

    void SymbolCombo::remove_duplicates(SymbolCombo::storage_t &data) {
        // Iterate forwards, looking for duplicates
        LexEqComparator lex_eq{};
        auto leading_iter = data.begin();
        auto lagging_iter = leading_iter;
        ++leading_iter;
        const auto last_iter = data.end();
        while (leading_iter != last_iter) {
            assert(lagging_iter <= leading_iter);
            assert(leading_iter <= last_iter);
            if (lex_eq(*lagging_iter, *leading_iter)) {
                lagging_iter->factor += leading_iter->factor;
            } else {
                ++lagging_iter;
                if (leading_iter != lagging_iter) {
                    // copy/move
                    *lagging_iter = *leading_iter;
                }
            }
            ++leading_iter;
        }
        ++lagging_iter;
        assert(lagging_iter <= leading_iter);
        assert(leading_iter <= last_iter);
        data.erase(lagging_iter, last_iter);
    }

    void SymbolCombo::remove_zeros(SymbolCombo::storage_t &data) {
        auto read_iter = data.begin();
        auto write_iter = data.begin();
        const auto last_iter = data.end();

        while (read_iter != last_iter) {
            assert(write_iter <= read_iter);
            if (approximately_zero(read_iter->factor) || (read_iter->id == 0)) {
                ++read_iter; // skip zeros
                continue;
            }

            if (read_iter != write_iter) {
                *write_iter = *read_iter;
            }

            ++write_iter;
            ++read_iter;
        }

        assert(write_iter <= read_iter);
        assert(read_iter <= last_iter);
        data.erase(write_iter, last_iter);
    }

    SymbolCombo::SymbolCombo(const Monomial& expr) {
        if (0 != expr.id) {
            this->data.emplace_back(expr);
        }
    }

    SymbolCombo::SymbolCombo(const std::map<symbol_name_t, double> &input) {
        data.reserve(input.size());
        for (const auto& pair : input) {
            data.emplace_back(pair.first, pair.second);
        }
    }


    SymbolCombo::operator Monomial() const {
        if (!this->is_monomial()) {
            std::stringstream errSS;
            errSS << "\"" << *this << "\" is not a monomial expression.";
            throw std::logic_error{errSS.str()};
        }

        // If empty, create a "zero"
        if (this->data.empty()) {
            return Monomial{0, 1.0};
        }

        // Otherwise, copy first (and only) element:
        return this->data[0];
    }


    SymbolCombo& SymbolCombo::operator*=(const std::complex<double> factor) noexcept {
        if (approximately_zero(factor)) {
            this->data.clear();
            return *this;
        }

        if (approximately_equal(factor, 1.0)) {
            return *this;
        }

        for (auto& entry : this->data) {
            entry.factor *= factor;
        }
        return *this;
    }

    bool SymbolCombo::operator==(const SymbolCombo &rhs) const noexcept {
        if (this->data.size() != rhs.data.size()) {
            return false;
        }
        for (size_t index = 0; index < this->data.size(); ++index) {
            if (this->data[index] != rhs.data[index]) {
                return false;
            }
        }
        return true;
    }

    bool SymbolCombo::fix_cc_in_place(const SymbolTable &symbols, bool make_canonical) noexcept {
        bool any_change = false;
        for (auto& elem: this->data) {
            assert(elem.id < symbols.size());
            auto& symbolInfo = symbols[elem.id];
            if (symbolInfo.is_hermitian()) {
                any_change = elem.conjugated || any_change;
                elem.conjugated = false;
            }
            if (symbolInfo.is_antihermitian() && elem.conjugated) {
                any_change = elem.conjugated || any_change;
                elem.factor *= -1;
                elem.conjugated = false;
            }
        }

        // If any changes made, scan for duplicates and zeros
        if (make_canonical && any_change) {
            if (this->data.size() > 1) {
                remove_duplicates(this->data);
            }
            remove_zeros(this->data);
        }

        return any_change;
    }

    bool SymbolCombo::conjugate_in_place(const SymbolTable& symbols) noexcept {
        bool any_conjugate = false;

        for (auto& elem: this->data) {
            assert(elem.id < symbols.size());
            auto& symbolInfo = symbols[elem.id];
            // k -> k*
            elem.factor = std::conj(elem.factor);
            if (symbolInfo.is_hermitian()) {
                continue;
            }

            if (symbolInfo.is_antihermitian()) {
                elem.factor = -elem.factor;
            } else {
                elem.conjugated = !elem.conjugated;
            }

            any_conjugate = true;
        }

        // Re-order so A < A*:
        if (any_conjugate && (this->data.size() > 1)) {
            auto iter = this->data.begin();
            auto next_iter = iter+1;
            while (next_iter != this->data.end()) {
                if (iter->id == next_iter->id) {
                    if (iter->conjugated && !next_iter->conjugated) {
                        std::swap(*iter, *next_iter);
                    }
                }
                ++iter;
                ++next_iter;
            }
        }
        return any_conjugate;
    }


    bool SymbolCombo::is_hermitian(const SymbolTable& symbols) const noexcept {

        const Monomial* last_symbol = nullptr;
        for (const auto& elem : this->data) {

            // Factors of 0 are always hermitian.
            if (elem.factor == 0.0) {
                continue;
            }

            assert(elem.id < symbols.size());
            const auto& symbolInfo = symbols[elem.id];

            // Adding a Hermitian term preserves Hermiticity
            if (symbolInfo.is_hermitian()) {
                // X, Y; where X is not Hermitian
                if (last_symbol != nullptr) {
                    return false;
                }
                last_symbol = nullptr;
                continue;
            }

            // Symbol /could/ have complex parts. Note: X < X* in ordering.
            if (elem.conjugated) {
                if (last_symbol == nullptr) {
                    // elem.factor != 0.0
                    return false;
                } else {
                    // "X, Y*"; meaning either X* was missed, or Y was missed...
                    if (last_symbol->id != elem.id) {
                        return false;
                    }
                }

                // Expect kX, k*X*
                if (last_symbol->factor != std::conj(elem.factor)) {
                    return false;
                }

                last_symbol = nullptr;
            } else {
                // X, Y; where X is not Hermitian
                if (last_symbol != nullptr) {
                    return false;
                }
                last_symbol = &elem;
            }
        }
        // Expecting, but did not find, X*
        if (last_symbol != nullptr) {
            return false;
        }

        return true;
    }

    bool SymbolCombo::is_conjugate(const SymbolTable& symbols, const SymbolCombo &other) const noexcept {
        if (this->data.size() != other.data.size()) {
            return false;
        }
        for (size_t index = 0, iMax = this->data.size(); index < iMax; ++index) {
            const auto& lhs_elem = this->data[index];
            const auto& rhs_elem = other.data[index];

            if (lhs_elem.id != rhs_elem.id) {
                return false;
            }
            assert(lhs_elem.id < symbols.size());
            const auto& symbolInfo = symbols[lhs_elem.id];

            // Zero is zero.
            if (lhs_elem.id == 0) {
                continue;
            }
            // Nothing else is zero.
            assert(!(symbolInfo.is_antihermitian() && symbolInfo.is_hermitian()));

            if (symbolInfo.is_hermitian()) {
                if (!approximately_equal(lhs_elem.factor, std::conj(rhs_elem.factor))) {
                    return false;
                }
                // no need to compare conjugation, symbol is real.
            } else if (symbolInfo.is_antihermitian()) {
                // Symbol is purely imaginary; so either A = -A, or A = A*.
                if (lhs_elem.factor == std::conj(rhs_elem.factor)) {
                    if (lhs_elem.conjugated == rhs_elem.conjugated) {
                        return false;
                    }
                } else if (lhs_elem.factor == -std::conj(rhs_elem.factor)) {
                    if (lhs_elem.conjugated != rhs_elem.conjugated) {
                        return false;
                    }
                } else {
                    return false;
                }
            }
        }

        return true;
    }


    std::ostream &operator<<(std::ostream &os, const SymbolCombo &combo) {
        const bool initial_plus_status = os.flags() & std::ios::showpos;
        const bool initial_show_base_status = os.flags() & std::ios::showbase;

        os.unsetf(std::ios::showpos);
        os.setf(std::ios::showbase);
        for (const auto& se : combo) {
            os << se;
            os.setf(std::ios::showpos); // 'done once'
        }

        // Restore initial "showpos" status
        if (initial_plus_status) {
            os.setf(std::ios::showpos);
        } else {
            os.unsetf(std::ios::showpos);
        }

        // Restore initial "showbase" status
        if (initial_show_base_status) {
            os.setf(std::ios::showbase);
        } else {
            os.unsetf(std::ios::showbase);
        }

        return os;
    }

    std::string SymbolCombo::as_string() const {

        std::stringstream ss;
        ss << *this;
        return ss.str();
    }


}