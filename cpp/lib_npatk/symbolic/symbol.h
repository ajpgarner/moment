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

}