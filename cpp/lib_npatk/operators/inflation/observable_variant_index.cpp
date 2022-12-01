/**
 * observable_variant_index.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "observable_variant_index.h"

#include "utilities/alphabetic_namer.h"

#include <iostream>


namespace NPATK {

    std::ostream& operator<<(std::ostream& os, const OVIndex& index) {
        AlphabeticNamer obsNamer{true};
        os << obsNamer(index.observable) << index.variant;
        return os;
    }
}