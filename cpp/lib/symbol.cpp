#include "symbol.h"

#include <iostream>

namespace NPATK {
    std::ostream &operator<<(std::ostream &os, const Symbol& symb) {
        if (symb.negated) {
            os << "-";
        }
        os << symb.id;
        if (symb.conjugated) {
            os << "*";
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const SymbolPair &pair) {
        os << pair.left_id << " == ";
        if (pair.negated) {
            os << "-";
        }
        os << pair.right_id;
        if (pair.conjugated) {
            os << "*";
        }
        return os;
    }
}