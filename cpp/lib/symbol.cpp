/**
 * symbol.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol.h"

#include <iostream>

namespace NPATK {

    std::ostream &operator<<(std::ostream &os, const Symbol &symb) {
        os << symb.id;
        if (symb.im_is_zero || symb.real_is_zero) {
            os << " [";
            if (symb.is_zero()) {
                os << "zero";
            } else if (symb.im_is_zero) {
                os << "real";
            } else {
                os << "imaginary";
            }
            os << "]";
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const SymbolExpression& symb) {
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