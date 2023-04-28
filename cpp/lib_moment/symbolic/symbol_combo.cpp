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
        struct LexLessComparator {
            bool operator()(const SymbolExpression &lhs, const SymbolExpression &rhs) const noexcept {
                if (lhs.id < rhs.id) {
                    return true;
                } else if (lhs.id > rhs.id) {
                    return false;
                }

                if (lhs.conjugated == rhs.conjugated) {
                    return false;
                }

                return !lhs.conjugated; // true implies lhs a, rhs a*

            }
        };

        struct LexEqComparator {
            bool operator()(const SymbolExpression &lhs, const SymbolExpression &rhs) const noexcept {
                if (lhs.id != rhs.id) {
                    return false;
                }
                if (lhs.conjugated != rhs.conjugated) {
                    return false;
                }
                return true;
            }
        };

        void remove_duplicates(SymbolCombo::storage_t &data) {
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

        void remove_zeros(SymbolCombo::storage_t &data) {

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
    }

    SymbolCombo::SymbolCombo(const SymbolExpression& expr) {
        if (0 != expr.id) {
            this->data.emplace_back(expr);
        }
    }

    SymbolCombo::SymbolCombo(SymbolCombo::storage_t input)
        : data{std::move(input)} {

        // No pruning/sorting etc. if zero or one elements
        if (this->data.size() > 1) {
            // Put orders in lexographic order
            std::sort(this->data.begin(), this->data.end(), LexLessComparator{});

            // Remove duplicates
            remove_duplicates(this->data);
        }

        remove_zeros(this->data);
    }

    SymbolCombo::SymbolCombo(const std::map<symbol_name_t, double> &input) {
        data.reserve(input.size());
        for (const auto& pair : input) {
            data.emplace_back(pair.first, pair.second);
        }
    }


    SymbolCombo::operator SymbolExpression() const {
        if (!this->is_monomial()) {
            std::stringstream errSS;
            errSS << "\"" << *this << "\" is not a monomial expression.";
            throw std::logic_error{errSS.str()};
        }

        // If empty, create a "zero"
        if (this->data.empty()) {
            return SymbolExpression{0, 1.0};
        }

        // Otherwise, copy first (and only) element:
        return this->data[0];
    }


    SymbolCombo& SymbolCombo::operator*=(const double factor) noexcept {
        if (factor == 0) {
            this->data.clear();
            return *this;
        }

        if (factor == 1.0) {
            return *this;
        }

        for (auto& entry : this->data) {
            entry.factor *= factor;
        }
        return *this;
    }

    SymbolCombo& SymbolCombo::operator+=(const SymbolCombo &rhs) {
        SymbolCombo& lhs = *this;

        // Get data iterators for RHS
        auto rhsIter = rhs.data.begin();
        const auto rhsEnd = rhs.data.end();

        // RHS is empty, nothing to do
        if (rhsIter == rhsEnd) {
            return *this;
        }

        // Get data iterators for LHS
        auto lhsIter = lhs.data.begin();
        const auto lhsEnd = lhs.data.end();

        // LHS is empty, copy RHS
        if (lhsIter == lhsEnd) {
            lhs.data.reserve(rhs.size());
            std::copy(rhs.data.cbegin(), rhs.data.cend(), lhs.data.begin());
            return *this;
        }

        // Copy and merge, maintaining ordering
        const LexLessComparator lex_less;
        storage_t output_data;
        while ((lhsIter != lhsEnd) || (rhsIter != rhsEnd)) {
            if ((rhsIter == rhsEnd) || ((lhsIter != lhsEnd) && lex_less(*lhsIter, *rhsIter))) {
                output_data.push_back(*lhsIter); // Copy element from LHS
                ++lhsIter;
            } else if ((lhsIter == lhsEnd) || ((rhsIter != rhsEnd) && lex_less(*rhsIter, *lhsIter))) {
                output_data.push_back(*rhsIter); // Copy element from RHS
                ++rhsIter;
            } else {
                assert(lhsIter != lhsEnd);
                assert(rhsIter != rhsEnd);
                assert(lhsIter->id == rhsIter->id);
                assert(lhsIter->conjugated == rhsIter->conjugated);

                const auto sumVals = lhsIter->factor + rhsIter->factor;

                if (!approximately_zero(sumVals)) {
                    output_data.emplace_back(lhsIter->id, sumVals, lhsIter->conjugated);
                }
                ++lhsIter;
                ++rhsIter;
            }
        }
        this->data.swap(output_data);
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

    SymbolCombo &SymbolCombo::conjugate_in_place(const SymbolTable& symbols) noexcept {
        bool any_conjugate = false;

        for (auto& elem: this->data) {
            assert(elem.id < symbols.size());
            auto& symbolInfo = symbols[elem.id];
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

        // Put into lexicographical order, if any non-trivial conjugations
        if (any_conjugate) {
            std::sort(this->data.begin(), this->data.end(), LexLessComparator{});
        }
        return *this;
    }


    bool SymbolCombo::is_hermitian(const SymbolTable& symbols) const noexcept {

        const SymbolExpression* last_symbol = nullptr;
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

                // Expect kX, kX*
                if (last_symbol->factor != elem.factor) {
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
                if (lhs_elem.factor != rhs_elem.factor) {
                    return false;
                }
                // no need to compare conjugation, symbol is real.
            } else if (symbolInfo.is_antihermitian()) {
                // Symbol is purely imaginary; so either A = -A, or A = A*.
                if (lhs_elem.factor == rhs_elem.factor) {
                    if (lhs_elem.conjugated == rhs_elem.conjugated) {
                        return false;
                    }
                } else if (lhs_elem.factor == -rhs_elem.factor) {
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