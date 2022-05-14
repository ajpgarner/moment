/**
 * symbol.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include <cstdint>
#include <cassert>

#include <vector>
#include <iosfwd>

namespace NPATK {

    using symbol_name_t = int_fast32_t;

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
        explicit Symbol(symbol_name_t name, bool complex = true) : id(name), im_is_zero(!complex) { }

        Symbol& merge_in(const Symbol& rhs) {
            this->real_is_zero |= rhs.real_is_zero;
            this->im_is_zero |= rhs.im_is_zero;
            return *this;
        }

        Symbol(const Symbol& rhs) = default;

         bool operator != (const Symbol&rhs) const {
             return (this->id != rhs.id)
                || (this->real_is_zero != rhs.real_is_zero)
                || (this->im_is_zero != rhs.im_is_zero);
         }

        friend std::ostream& operator<<(std::ostream& os, const Symbol& symb);
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
        symbol_name_t id;
        bool negated;
        bool conjugated;

    public:
        explicit SymbolExpression(symbol_name_t name, bool conj = false)
            : id(name), negated(name < 0), conjugated(conj) {
            if (id < 0) {
                id = -id;
            }
        }

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
        SymbolPair(SymbolExpression left, SymbolExpression right) {
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

        friend std::ostream& operator<<(std::ostream& os, const SymbolPair& pair);
    };

}