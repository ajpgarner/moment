/**
 * symbol.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol.h"

#include <iostream>
#include <string>

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
}