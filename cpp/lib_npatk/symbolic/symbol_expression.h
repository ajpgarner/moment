/**
 * symbol_expression.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "symbol.h"
#include <iosfwd>

namespace NPATK {
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
        bool negated;
        bool conjugated;

    public:
        constexpr explicit SymbolExpression(symbol_name_t name, bool conj = false) noexcept
                : id(name), negated(name < 0), conjugated(conj) {
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
                : id(name), negated(neg), conjugated(conj) { }

        bool operator==(const SymbolExpression& rhs) const {
            return (this->id == rhs.id)
                   && (this->negated == rhs.negated)
                   && (this->conjugated == rhs.conjugated);
        }

        bool operator!=(const SymbolExpression&rhs) const {
            return (this->id != rhs.id)
                   || (this->negated != rhs.negated)
                   || (this->conjugated != rhs.conjugated);
        }

        friend std::ostream& operator<<(std::ostream& os, const SymbolExpression& symb);

        /**
         * The maximum length string that we are willing to attempt to parse as an SymbolExpression.
         */
        const static size_t max_strlen = 32;

        /**
         * Gets the symbol expression as a signed integer. This ignores conjugation!
         */
        [[nodiscard]] constexpr std::make_signed_t<symbol_name_t> as_integer() const noexcept {
            return static_cast<std::make_signed_t<symbol_name_t>>(this->id) * (this->negated ? -1 : 1);
        }

        /**
         * Gets the symbol expression as a string.
         */
        [[nodiscard]] constexpr std::string as_string() const {
            return std::string(this->negated ? "-" : "") + std::to_string(this->id) + (this->conjugated ? "*" : "");
        }
    };


    /**
     * Represents equality between two symbols, potentially with negation and/or complex-conjugation.
     */
    struct SymbolPair {
    public:
        symbol_name_t left_id;
        symbol_name_t right_id;
        bool negated;
        bool conjugated;

    public:
        SymbolPair(SymbolExpression left, SymbolExpression right) noexcept {
            if (left.id <= right.id) {
                this->left_id = left.id;
                this->right_id = right.id;
            } else {
                this->left_id = right.id;
                this->right_id = left.id;
            }
            this->negated = left.negated ^ right.negated;
            this->conjugated = left.conjugated ^ right.conjugated;
        }

        SymbolPair(symbol_name_t left_id, symbol_name_t right_id, bool neg, bool conj) noexcept  {
            if (left_id <= right_id) {
                this->left_id = left_id;
                this->right_id = right_id;
            } else {
                this->left_id = right_id;
                this->right_id = left_id;
            }
            this->negated = neg;
            this->conjugated = conj;
        }

        friend std::ostream& operator<<(std::ostream& os, const SymbolPair& pair);
    };
}