/**
 * symbol_expression.h
 *
 * Copyright (c) 2022-2023 Austrian Academy of Sciences
 */
#pragma once

#include "symbol.h"

#include <iosfwd>

namespace Moment {
    /**
     * An algebraic element, as might be written in a matrix or equation.
     * Effectively, a symbol up to some pre-factor and conjugation.
     */
    struct SymbolExpression {
    public:
        /**
         * Error thrown when string expression cannot be parsed as a symbol expression.
         */
        struct SymbolParseException : public std::runtime_error {
        public:
            /** Underlying exception, if any */
            std::optional<std::exception> cause;

        public:
            explicit SymbolParseException (const std::string &badExpr) :
                    std::runtime_error(make_msg(badExpr)) { }

            explicit SymbolParseException (const std::string &badExpr, const std::exception& e) :
                    std::runtime_error(make_msg(badExpr, e)), cause{e} { }

            static std::string make_msg(const std::string &badExpr);

            static std::string make_msg(const std::string &badExpr, const std::exception& e);

        };

    public:
        symbol_name_t id;
        double factor;
        bool conjugated;

    public:
        /**
         * Default (uninitialized!) construction of SymbolExpression
         */
        constexpr explicit SymbolExpression() = default;

        /**
         * Construct a symbol expression.
         * @param name The symbol ID. Use a negative value to interpret as -1 times absolute value.
         * @param conj Whether the symbol is conjugated.
         */
        constexpr explicit SymbolExpression(std::make_signed<symbol_name_t>::type name, bool conj = false) noexcept
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
        constexpr explicit SymbolExpression(symbol_name_t name, double factor, bool conj = false) noexcept
                : id{name}, factor{factor}, conjugated{conj} { }

        /**
         * Construct a symbol expression.
         * @param name The symbol ID.
         * @param neg Whether the symbol is negated.
         * @param conj Whether the symbol is conjugated.
         */
        constexpr SymbolExpression(symbol_name_t name, bool neg, bool conj) noexcept
                : id(name), factor{neg ? -1.0 : 1.0}, conjugated(conj) { }

        /**
         * Construct a symbol expression, from supplied string input.
         * @param strExpr String representing the expression
         * @throws SymbolParseException if strExpr cannot be interpreted as a valid symbol.
         */
        explicit SymbolExpression(const std::string& strExpr);

        [[nodiscard]] bool operator==(const SymbolExpression& rhs) const {
            return (this->id == rhs.id)
                   && (this->factor == rhs.factor)
                   && ((this->id == 0) || (this->conjugated == rhs.conjugated));
        }

        [[nodiscard]] bool operator!=(const SymbolExpression&rhs) const {
            return (this->id != rhs.id)
                   || (this->factor != rhs.factor)
                   || ((this->id != 0) && (this->conjugated != rhs.conjugated));
        }

        /**
         * The maximum length string that we are willing to attempt to parse as an SymbolExpression.
         */
        const static size_t max_strlen = 32;

        /**
         * Gets the symbol expression as a signed integer. This ignores conjugation, and factors!
         */
        [[nodiscard]] constexpr std::make_signed_t<symbol_name_t> as_integer() const noexcept {
            return static_cast<std::make_signed_t<symbol_name_t>>(this->id) * ((this->factor < 0) ? -1 : 1);
        }

        /**
         * True if the symbol has a negative factor
         */
        [[nodiscard]] constexpr bool negated() const noexcept { return this->factor < 0; }

        /**
         * Gets the symbol expression as a string.
         */
        [[nodiscard]] std::string as_string() const;

        friend std::ostream& operator<<(std::ostream& os, const SymbolExpression& expr);
    };
}