/**
 * operator.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <cstdint>

#include "integer_types.h"

namespace NPATK {

    struct Operator {
    public:
        enum class Flags : uint8_t {
            /** No flags set */
            None = 0x00,
            /** Operator is the identity element */
            Identity = 0x01,
            /** Operator X^2 = X */
            Idempotent = 0x02
        };

        friend constexpr Flags operator|(Flags lhs, Flags rhs) noexcept {
            return static_cast<Flags>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
        }

        friend constexpr Flags operator&(Flags lhs, Flags rhs) noexcept {
            return static_cast<Flags>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
        }

    public:
        /**
         * Predicate: true if the party of LHS is less than that of RHS.
         */
        struct PartyComparator {
            constexpr bool operator()(const Operator &lhs, const Operator &rhs) const noexcept {
                return lhs.party < rhs.party;
            }
        };

        /**
         * Predicate: true if lhs = rhs, and lhs is idempotent.
         * I.e., true if 'AB' can be replaced by 'A'.
         */
        struct IsRedundant {
            constexpr bool operator()(const Operator &lhs, const Operator &rhs) const noexcept {
                return lhs.idempotent() && (lhs == rhs);
            }
        };

    public:
        /** Identifier of operator */
        oper_name_t id;

        /** Group of (potentially) non-commuting operators. Operators between parties always commute. */
        party_name_t party;

        /** Operator flags */
        Flags flags;

        explicit constexpr Operator(oper_name_t name, party_name_t who = 0, Flags what = Flags::None) noexcept
                : id{name}, party{who}, flags{what} { }

        constexpr bool operator==(const Operator &rhs) const noexcept {
            return (this->id == rhs.id) && (this->party == rhs.party);
            // undefined to have same party & id, w/ different flags && (this->flags == rhs.flags);
        }

        /** True if X^2 = X */
        [[nodiscard]] constexpr bool idempotent() const noexcept {
            return (this->flags & Flags::Idempotent) == Flags::Idempotent;
        }

        /** True if XY = Y for all Y */
        [[nodiscard]] constexpr bool identity() const noexcept {
            return (this->flags & Flags::Identity) == Flags::Identity;
        }
    };
}