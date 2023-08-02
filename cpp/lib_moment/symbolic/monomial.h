/**
 * monomial.h
 *
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include "scenarios/contextual_os.h"

#include "utilities/float_utils.h"

#include <complex>
#include <iosfwd>
#include <optional>

namespace Moment {
    /**
     * An algebraic element, as might be written in a matrix or equation.
     * Effectively, a symbol up to some pre-factor and conjugation.
     */
    struct Monomial {
    public:
        /**
         * Error thrown when string expression cannot be parsed as a symbol expression.
         */
        struct SymbolParseException : public std::runtime_error {
        public:
            /** Underlying exception, if any */
            std::optional<std::exception> cause;

        public:
            explicit SymbolParseException(const std::string &badExpr) :
                    std::runtime_error(make_msg(badExpr)) {}

            explicit SymbolParseException(const std::string &badExpr, const std::exception &e) :
                    std::runtime_error(make_msg(badExpr, e)), cause{e} {}

            static std::string make_msg(const std::string &badExpr);

            static std::string make_msg(const std::string &badExpr, const std::exception &e);

        };

    public:
        symbol_name_t id;
        std::complex<double> factor;
        bool conjugated;

    public:
        /**
         * Default (uninitialized!) construction of Monomial
         */
        constexpr explicit Monomial() = default;

        /**
         * Construct a symbol expression.
         * @param name The symbol ID. Use a negative value to interpret as -1 times absolute value.
         * @param conj Whether the symbol is conjugated.
         */
        constexpr explicit Monomial(std::make_signed<symbol_name_t>::type name, bool conj = false) noexcept
                : id{name}, factor{(name < 0) ? -1.0 : 1.0}, conjugated{conj} {
            if (id < 0) {
                id = -id;
            }
        }

        /**
         * Construct a symbol expression.
         * @param name The symbol ID.
         * @param neg The scalar factor for this symbol.
         * @param conj Whether the symbol is conjugated.
         */
        constexpr explicit Monomial(symbol_name_t name, double factor, bool conj = false) noexcept
                : id{name}, factor{factor, 0.0}, conjugated{conj} {}

        /**
         * Construct a symbol expression.
         * @param name The symbol ID.
         * @param neg The scalar factor for this symbol.
         * @param conj Whether the symbol is conjugated.
         */
        constexpr explicit Monomial(symbol_name_t name, std::complex<double> factor, bool conj = false) noexcept
                : id{name}, factor{factor}, conjugated{conj} {}

        /**
         * Construct a symbol expression.
         * @param name The symbol ID.
         * @param neg Whether the symbol is negated.
         * @param conj Whether the symbol is conjugated.
         */
        constexpr Monomial(symbol_name_t name, bool neg, bool conj) noexcept
                : id(name), factor{neg ? -1.0 : 1.0}, conjugated(conj) {}

        /**
         * Construct a symbol expression, from supplied string input.
         * @param strExpr String representing the expression
         * @throws SymbolParseException if strExpr cannot be interpreted as a valid symbol.
         */
        explicit Monomial(const std::string &strExpr);


        [[nodiscard]] bool approximately_equals(const Monomial &rhs, double eps_multiplier = 1.0) const noexcept {
            return (this->id == rhs.id)
                   && ((this->id == 0) || ((this->conjugated == rhs.conjugated)
                                           && approximately_equal(this->factor, rhs.factor, eps_multiplier)));
        }

        [[nodiscard]] bool not_approximately_equals(const Monomial &rhs, double eps_multiplier = 1.0) const noexcept {
            return (this->id != rhs.id)
                   || ((this->id != 0) && ((this->conjugated != rhs.conjugated)
                                           || !approximately_equal(this->factor, rhs.factor, eps_multiplier)));
        }


        [[nodiscard]] inline bool operator==(const Monomial &rhs) const noexcept {
            return this->approximately_equals(rhs);
        }

        [[nodiscard]] inline bool operator!=(const Monomial &rhs) const noexcept {
            return this->not_approximately_equals(rhs);
        }

        /**
         * The maximum length string that we are willing to attempt to parse as an Monomial.
         */
        const static size_t max_strlen = 32;

        /**
         * True if the symbol has a complex factor.
         */
        [[nodiscard]] constexpr bool complex_factor() const noexcept {
            return !approximately_real(this->factor);
        }

        /**
         * True if the symbol has a negative factor. (False if factor is complex.)
         */
        [[nodiscard]] constexpr bool negated() const noexcept {
            return approximately_real(this->factor) && (this->factor.real() < 0);
        }

        friend std::ostream& operator<<(std::ostream& os, const Monomial &expr);

        friend ContextualOS& operator<<(ContextualOS& os, const Monomial &expr);

        std::string as_string(const StringFormatContext& format_context) const;

        void format_as_symbol_id_without_context(std::ostream& os,
                                                 bool show_plus = false,
                                                 bool show_hash = true) const;

        inline void format_as_symbol_id_with_context(ContextualOS& os) const {
            this->format_as_symbol_id_without_context(os.os, !os.format_info.first_in_polynomial,
                                                      os.format_info.hash_before_symbol_id);
        }

        void format_as_operator_sequence_with_context(ContextualOS& os) const;

        friend class Polynomial;
    };
}