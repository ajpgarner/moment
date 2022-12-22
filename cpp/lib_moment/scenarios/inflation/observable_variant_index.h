/**
 * observable_variant_index.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"

#include <iosfwd>

namespace Moment::Inflation {
    struct OVIndex {
    public:
        oper_name_t observable;
        oper_name_t variant;

        constexpr OVIndex() = default;

        constexpr OVIndex(oper_name_t o, oper_name_t v) : observable{o}, variant{v} { }

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

    struct OVOIndex {
        OVIndex observable_variant;
        oper_name_t outcome;

        constexpr OVOIndex() = default;

        constexpr OVOIndex(OVIndex obs_var, oper_name_t out)
                : observable_variant{obs_var}, outcome{out} { }

        constexpr OVOIndex(oper_name_t obs, oper_name_t var, oper_name_t out)
            : observable_variant{obs, var}, outcome{out} { }

        constexpr bool operator==(const OVOIndex& other) const noexcept {
            return (this->observable_variant == other.observable_variant) && (this->outcome == other.outcome);
        }

        constexpr bool operator<(const OVOIndex& other) const noexcept {
            if (this->observable_variant.observable < other.observable_variant.observable) {
                return true;
            } else if (this->observable_variant.observable > other.observable_variant.observable) {
                return false;
            }
            if (this->observable_variant.variant < other.observable_variant.variant) {
                return true;
            } else if (this->observable_variant.variant > other.observable_variant.variant) {
                return false;
            }
            return this->outcome < other.outcome;
        }

        friend std::ostream& operator<<(std::ostream& os, const OVOIndex& index);
    };
}