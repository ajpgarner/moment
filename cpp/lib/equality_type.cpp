/**
 * equality_type.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "equality_type.h"
#include <iostream>

namespace NPATK {

    std::ostream &operator<<(std::ostream &os, EqualityType et) {
        bool once = false;
        if ((et & EqualityType::equal) == EqualityType::equal) {
            os << "Equal";
            once = true;
        }
        if ((et & EqualityType::negated) == EqualityType::negated) {
            if (once) {
                os << " | ";
            }
            os << "Negated";
            once = true;
        }
        if ((et & EqualityType::conjugated) == EqualityType::conjugated) {
            if (once) {
                os << " | ";
            }
            os << "Conjugated";
            once = true;
        }
        if ((et & EqualityType::neg_conj) == EqualityType::neg_conj) {
            if (once) {
                os << " | ";
            }
            os << "Neg-conjugated";
            once = true;
        }
        if (!once) {
            os << "None";
        }
        return os;
    }
}
