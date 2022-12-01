/**
 * observable_variant_index.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"

#include <iosfwd>

namespace NPATK {
    struct OVIndex {
    public:
        oper_name_t observable;
        oper_name_t variant;

        OVIndex() = default;

        OVIndex(oper_name_t o, oper_name_t v) : observable{o}, variant{v} { }

        constexpr bool operator<(const OVIndex& other) const noexcept {
            if (this->observable < other.observable) {
                return true;
            } else if (this->observable > other.observable) {
                return false;
            }
            return this->variant < other.variant;
        }

        constexpr bool operator==(const OVIndex& other) const noexcept {
            return (this->observable == other.observable) && (this->variant == other.variant);
        }

        friend std::ostream& operator<<(std::ostream& os, const OVIndex& index);
    };
}