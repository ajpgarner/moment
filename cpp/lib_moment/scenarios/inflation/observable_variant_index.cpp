/**
 * observable_variant_index.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "observable_variant_index.h"

#include "utilities/alphabetic_namer.h"

#include <iostream>


namespace Moment {

    std::ostream& operator<<(std::ostream& os, const OVIndex& index) {
        AlphabeticNamer obsNamer{true};
        os << obsNamer(index.observable) << index.variant;
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const OVOIndex& index) {
        os << index.observable_variant << "." << index.outcome;
        return os;
    }
}