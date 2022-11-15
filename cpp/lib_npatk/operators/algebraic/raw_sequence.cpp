/**
 * raw_sequence.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "raw_sequence.h"

#include <iostream>

namespace NPATK {
    std::ostream& operator<<(std::ostream& os, const RawSequence& rs) {
        if (rs.operators.empty()) {
            if (rs.zero()) {
                os << "0";
            } else {
                os << "I";
            }
        } else {
            for (const auto& o : rs.operators) {
                os << "X" << o;
            }
        }
        return os;
    }

}