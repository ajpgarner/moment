#pragma once
#include <cstdint>
#include <cassert>

#include <vector>
#include <iosfwd>

namespace NPATK {

    using symbol_name_t = int_fast32_t;

    /**
     * An algebraic element.
     */
    struct Symbol {
    public:
        symbol_name_t id;
        bool negated;
        bool conjugated;

    public:
        explicit Symbol(symbol_name_t name, bool conj = false)
            : id(name), negated(name < 0), conjugated(conj) {
            if (id < 0) {
                id = -id;
            }
        }

        bool operator==(const Symbol& rhs) const {
            return (this->id == rhs.id)
                && (this->negated == rhs.negated)
                && (this->conjugated == rhs.conjugated);
        }

        bool operator!=(const Symbol&rhs) const {
            return (this->id != rhs.id)
                || (this->negated != rhs.negated)
                || (this->conjugated != rhs.conjugated);
        }

        friend std::ostream& operator<<(std::ostream& os, const Symbol& symb);
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
        SymbolPair(Symbol left, Symbol right) {
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