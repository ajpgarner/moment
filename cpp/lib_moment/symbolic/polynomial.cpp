/**
 * polynomial.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "polynomial.h"
#include "polynomial_factory.h"
#include "symbol_table.h"

#include "utilities/float_utils.h"
#include "utilities/format_factor.h"

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

    void Polynomial::remove_duplicates(Polynomial::storage_t &data) {
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

    void Polynomial::remove_zeros(Polynomial::storage_t &data, double eps_multiplier) {
        auto read_iter = data.begin();
        auto write_iter = data.begin();
        const auto last_iter = data.end();

        while (read_iter != last_iter) {
            assert(write_iter <= read_iter);
            if (approximately_zero(read_iter->factor, eps_multiplier) || (read_iter->id == 0)) {
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

    Polynomial::Polynomial(const Monomial& expr) {
        if (0 != expr.id) {
            this->data.emplace_back(expr);
        }
    }

    Polynomial::Polynomial(const std::map<symbol_name_t, double> &input) {
        data.reserve(input.size());
        for (const auto& pair : input) {
            data.emplace_back(pair.first, pair.second);
        }
    }


    Polynomial::operator Monomial() const {
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


    Polynomial& Polynomial::operator*=(const std::complex<double> factor) noexcept {
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


    bool Polynomial::approximately_equals(const Polynomial &rhs, double eps_multiplier) const noexcept {
        if (this->data.size() != rhs.data.size()) {
            return false;
        }
        for (size_t index = 0; index < this->data.size(); ++index) {
            if (!this->data[index].approximately_equals(rhs.data[index], eps_multiplier)) {
                return false;
            }
        }
        return true;
    }

    bool Polynomial::fix_cc_in_place(const SymbolTable &symbols, bool make_canonical, double zero_tolerance) noexcept {
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
            remove_zeros(this->data, zero_tolerance);
        }

        return any_change;
    }

    bool Polynomial::conjugate_in_place(const SymbolTable& symbols) noexcept {
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


    bool Polynomial::is_hermitian(const SymbolTable& symbols, double tolerance) const noexcept {
        const Monomial* last_symbol = nullptr;
        for (const auto& elem : this->data) {

            // Factors of 0 are always hermitian (but evil...)
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

                // Factor must be real, for reality.
                if (!approximately_real(elem.factor, tolerance)) {
                    return false;
                }
                continue;
            } else if (symbolInfo.is_antihermitian()) {
                // X, Y; where X is not Hermitian
                if (last_symbol != nullptr) {
                    return false;
                }
                last_symbol = nullptr;

                // Factor must be imaginary, for reality.
                if (!approximately_imaginary(elem.factor, tolerance)) {
                    return false;
                }
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
                if (!approximately_equal(last_symbol->factor, std::conj(elem.factor), tolerance)) {
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

    bool Polynomial::is_antihermitian(const SymbolTable& symbols, double tolerance) const noexcept {
        const Monomial* last_symbol = nullptr;
        for (const auto& elem : this->data) {

            // Factors of 0 are always anti-hermitian (but evil...)
            if (elem.factor == 0.0) {
                continue;
            }

            assert(elem.id < symbols.size());
            const auto& symbolInfo = symbols[elem.id];

            // Adding a Hermitian term preserves Hermiticity
            if (symbolInfo.is_antihermitian()) {
                // X, Y; where X is not anti-Hermitian
                if (last_symbol != nullptr) {
                    return false;
                }
                last_symbol = nullptr;

                // Factor must be real, for overall anti-Hermitianness
                if (!approximately_real(elem.factor, tolerance)) {
                    return false;
                }
                continue;
            } else if (symbolInfo.is_hermitian()) {
                // X, Y; where X is not anti-Hermitian
                if (last_symbol != nullptr) {
                    return false;
                }
                last_symbol = nullptr;

                // Factor must be imaginary, for overall anti-Hermitianness
                if (!approximately_imaginary(elem.factor, tolerance)) {
                    return false;
                }
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

                // Expect kX, -k*X*
                if (!approximately_equal(last_symbol->factor, -std::conj(elem.factor), tolerance)) {
                    return false;
                }

                last_symbol = nullptr;
            } else {
                // X, Y; where X is not anti-Hermitian
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

    bool Polynomial::is_conjugate(const SymbolTable& symbols, const Polynomial &other) const noexcept {
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

    Polynomial Polynomial::Real(const PolynomialFactory &factory) const {
        if (this->data.empty()) {
            return Polynomial::Zero();
        }

        // Prepare for output
        Polynomial::storage_t output_storage;

        // Iterate
        auto iter = this->data.begin();
        while (iter != this->data.end()) {
            const symbol_name_t id = iter->id;
            // Three cases: X alone, X* alone, X, and X* together.
            const std::complex<double> factor = iter->factor;

            std::complex<double> output_factor, output_conj_factor;
            if (!iter->conjugated) {
                auto conjugate_factor = [&]() -> std::optional<std::complex<double>> {
                    auto peek_iter = iter + 1;
                    if (peek_iter != this->data.end()) {
                        if (iter->id == peek_iter->id) {
                            return peek_iter->factor;
                        }
                    }
                    return std::nullopt;
                }();
                if (!conjugate_factor.has_value()) {
                    // Case: X alone
                    output_factor = std::complex{0.5, 0.0} * factor;
                    output_conj_factor = std::complex{0.5, 0.0} * std::conj(factor);
                    ++iter;
                } else {
                    // Case: X + X*
                    output_factor = std::complex{0.5, 0.0} * (factor + std::conj(conjugate_factor.value()));
                    output_conj_factor = std::complex{0.5, 0.0} * (std::conj(factor) + conjugate_factor.value());
                    ++iter;
                    ++iter;
                }
            } else {
                // Case: X* alone [factor effectively acts as conjugate_factor]
                output_factor = std::complex{0.5, 0.0} * std::conj(factor);
                output_conj_factor = std::complex{0.5, 0.0} * factor;

                ++iter;
            }

            if (!approximately_zero(output_factor, factory.zero_tolerance)) {
                output_storage.emplace_back(id, output_factor, false);
            }
            if (!approximately_zero(output_conj_factor, factory.zero_tolerance)) {
                output_storage.emplace_back(id, output_conj_factor, true);
            }
        }
        return factory(std::move(output_storage));
    }

    Polynomial Polynomial::Imaginary(const PolynomialFactory &factory) const {
        if (this->data.empty()) {
            return Polynomial::Zero();
        }

        // Prepare for output
        Polynomial::storage_t output_storage;

        // Iterate
        auto iter = this->data.begin();
        while (iter != this->data.end()) {
            const symbol_name_t id = iter->id;
            // Three cases: X alone, X* alone, X, and X* together.
            const std::complex<double> factor = iter->factor;

            std::complex<double> output_factor, output_conj_factor;
            if (!iter->conjugated) {
                auto conjugate_factor = [&]() -> std::optional<std::complex<double>> {
                    auto peek_iter = iter + 1;
                    if (peek_iter != this->data.end()) {
                        if (iter->id == peek_iter->id) {
                            return peek_iter->factor;
                        }
                    }
                    return std::nullopt;
                }();
                if (!conjugate_factor.has_value()) {
                    // Case: X alone
                    output_factor = std::complex{0.0, -0.5} * factor;
                    output_conj_factor = std::complex{0.0, 0.5} * std::conj(factor);
                    ++iter;
                } else {
                    // Case: X + X*
                    output_factor = std::complex{0.0, -0.5} * (factor - std::conj(conjugate_factor.value()));
                    output_conj_factor = std::complex{0.0 ,0.5} * (std::conj(factor) - conjugate_factor.value());
                    ++iter;
                    ++iter;
                }
            } else {
                // Case: X* alone [factor effectively acts as conjugate_factor]
                output_factor = std::complex{0.0, 0.5} * std::conj(factor);
                output_conj_factor = std::complex{0.0, -0.5} * factor;

                ++iter;
            }


            if (!approximately_zero(output_factor, factory.zero_tolerance)) {
                output_storage.emplace_back(id, output_factor, false);
            }
            if (!approximately_zero(output_conj_factor, factory.zero_tolerance)) {
                output_storage.emplace_back(id, output_conj_factor, true);
            }
        }
        return factory(std::move(output_storage));
    }


    std::ostream &operator<<(std::ostream &os, const Polynomial &combo) {
        const bool initial_plus_status = os.flags() & std::ios::showpos;
        const bool initial_show_base_status = os.flags() & std::ios::showbase;

        // If empty, just write "0" and be done.
        if (combo.empty()) {
            if (initial_plus_status) {
                os << " + ";
            }
            os << "0";
            return os;
        }

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

    std::string Polynomial::as_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

    std::string Polynomial::as_string_with_operators(const SymbolTable &table, bool show_braces) const {
        std::stringstream ss;
        this->as_string_with_operators(ss, table, show_braces);
        return ss.str();
    }

    void Polynomial::as_string_with_operators(std::ostream &os, const SymbolTable &table, bool show_braces) const {
        // Empty string is always 0.
        if (this->empty()) {
            os << "0";
            return;
        }

        bool done_once = false;
        for (const auto& elem : this->data) {

            // Zero
            if ((elem.id == 0) || (approximately_zero(elem.factor))) {
                if (done_once) {
                    os << " + ";
                }
                os << "0";
                done_once = true;
                continue;
            }

            // Is element a scalar?
            const bool is_scalar = (elem.id == 1);

            // Write factor
            const bool need_space = format_factor(os, elem.factor, is_scalar, done_once);
            done_once = true;

            // Scalar, factor alone is enough
            if (is_scalar) {
                continue;
            }

            if (need_space) {
                os << " ";
            }

            // Skip if symbol not in table.
            const bool valid_symbol = ((elem.id >= 0) && (elem.id < table.size()));
            if (!valid_symbol) {
                os << "UNK#" << elem.id;
                continue;
            }

            // Get formatted sequence from within table.
            const auto& symbol_info = table[elem.id];
            if (show_braces) {
                if (elem.conjugated) {
                    os << "<" << symbol_info.formatted_sequence_conj() << ">";
                } else {
                    os << "<" << symbol_info.formatted_sequence() << ">";
                }
            } else {
                if (elem.conjugated) {
                    os << symbol_info.formatted_sequence_conj();
                } else {
                    os << symbol_info.formatted_sequence();
                }
            }
            done_once = true;
        }


    }


}