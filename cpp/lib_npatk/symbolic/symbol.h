/**
 * symbol.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include <cstdint>
#include <cassert>

#include <iosfwd>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace NPATK {

    using symbol_name_t = int64_t;

    /**
     * An algebraic element
     */
    struct Symbol {
        /** Unique identifier for this algebraic element */
        symbol_name_t id;

        /** True if Re(Symbol) = 0; i.e. symbol is imaginary or zero. */
        bool real_is_zero = false;

        /** True if Im(Symbol) = 0; i.e. symbol is real or zero. */
        bool im_is_zero = false;

        /** True if real and imaginary parts of symbol must be zero */
        [[nodiscard]] bool is_zero() const {
            return real_is_zero && im_is_zero;
        }

        /**
         * Construct algebraic element.
         * @param name The identifier for the algebraic element
         * @param complex True if Symbol could be a complex value; false if real-valued.
         */
        explicit constexpr Symbol(symbol_name_t name, bool complex = true) : id(name), im_is_zero(!complex) { }


        /**
         * Construct algebraic element.
         * @param name The identifier for the algebraic element
         * @param re_zero True if Symbol does not have a real component.
         * @param im_zero True if Symbol does not have an imaginary component.
         */
        constexpr Symbol(symbol_name_t name, bool re_zero, bool im_zero)
            : id(name), real_is_zero(re_zero), im_is_zero(im_zero) { }

        constexpr Symbol& merge_in(const Symbol& rhs) {
            this->real_is_zero |= rhs.real_is_zero;
            this->im_is_zero |= rhs.im_is_zero;
            return *this;
        }

        constexpr Symbol(const Symbol& rhs) = default;

        constexpr bool operator != (const Symbol&rhs) const {
             return (this->id != rhs.id)
                || (this->real_is_zero != rhs.real_is_zero)
                || (this->im_is_zero != rhs.im_is_zero);
         }

        friend std::ostream& operator<<(std::ostream& os, const Symbol& symb);

        static constexpr Symbol zero() noexcept {
            return Symbol{0, true, true};
        }
    };

    /**
     * Functor, returns true if left Symbol's ID is less than right Symbol's.
     */
    struct SymbolNameCompare {
        bool operator() (const Symbol& lhs, const Symbol& rhs) const noexcept {
            return lhs.id < rhs.id;
        }
    };

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