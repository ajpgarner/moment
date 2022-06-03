/**
 * operator.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <cinttypes>
#include <iosfwd>


namespace NPATK {

    using party_name_t = int16_t;

    struct Party {
        party_name_t id;

        constexpr explicit Party(party_name_t party_id) noexcept: id{party_id} {}

        constexpr bool operator<(const Party &rhs) const noexcept {
            return this->id < rhs.id;
        }

        constexpr bool operator==(const Party &rhs) const noexcept {
            return this->id == rhs.id;
        }

        constexpr bool operator!=(const Party &rhs) const noexcept {
            return this->id != rhs.id;
        }
    };


    using oper_name_t = int64_t;

    struct Operator {
    public:
        enum class Flags : uint8_t {
            /** No flags set */
            None = 0x00,
            /** Operator is the identity element */
            Identity = 0x01,
            /** Operator X^2 = X */
            Idempotent = 0x02,
            /** Operator is zero element */
            Zero = 0x04
        };

        friend constexpr Flags operator|(Flags lhs, Flags rhs) noexcept {
            return static_cast<Flags>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
        }

        friend constexpr Flags operator&(Flags lhs, Flags rhs) noexcept {
            return static_cast<Flags>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
        }


    public:
        /**
         * Predicate: true if the party ID of LHS is less than that of RHS.
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
        oper_name_t id;
        Party party;
        Flags flags;

        constexpr Operator(oper_name_t name, Party who, Flags what = Flags::None) noexcept
                : id{name}, party{who}, flags{what} {}


        friend std::ostream &operator<<(std::ostream &os, const Operator &seq);

        constexpr bool operator==(const Operator &rhs) const noexcept {
            return (this->id == rhs.id) && (this->party == rhs.party);
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