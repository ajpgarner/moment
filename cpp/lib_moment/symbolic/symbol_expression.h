/**
 * symbol_expression.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "symbol.h"

#include <iosfwd>

namespace Moment {
    /**
        * An algebraic element, as might be written in a matrix or equation.
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
        constexpr explicit SymbolExpression() = default;

        constexpr explicit SymbolExpression(symbol_name_t name, bool conj = false) noexcept
                : id{name}, factor{(name < 0) ? -1.0 : 1.0}, conjugated{conj} {
            if (id < 0) {
                id = -id;
            }
        }

        /**
         * Construct a symbol expression, from supplied string input.
         * @param strExpr String representing the expression
         * @throws SymbolParseException if strExpr cannot be interpreted as a valid symbol.
         */
        explicit SymbolExpression(const std::string& strExpr);

        constexpr SymbolExpression(symbol_name_t name, bool neg, bool conj) noexcept
                : id(name), factor{neg ? -1.0 : 1.0}, conjugated(conj) { }

        bool operator==(const SymbolExpression& rhs) const {
            return (this->id == rhs.id)
                   && (this->factor == rhs.factor)
                   && ((this->id == 0) || (this->conjugated == rhs.conjugated));
        }

        bool operator!=(const SymbolExpression&rhs) const {
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